/*
 * Parser.cpp — ISO C (C11) recursive-descent parser.
 *
 * Every method maps 1-to-1 to a rule in the ISO grammar document:
 *   §1 Expressions  §2 Declarations  §3 Declarators
 *   §4 Initializers §5 Statements    §6 External Definitions
 *
 * Left-recursion is eliminated via iterative while-loops.
 * Left-factoring is handled by checking the lookahead token.
 * Typedef names are tracked so "typedef-name" resolves correctly.
 */

#include "parser.h"
#include <stdexcept>
#include <iostream>

// ============================================================
// printParseTree — verbatim concrete parse tree dump.
//
// Prints every node exactly as the parser constructed it,
// including all grammar-artifact wrappers (BlockItemList,
// BlockItem, InitDeclaratorList, PrimaryExpression, …).
// For the simplified AST view use printAST() from ast.h/cpp.
// ============================================================
void printParseTree(const shared_ptr<ASTNode>& node, int depth) {
    if (!node) return;

    string indent(depth * 2, ' ');
    string branch = depth > 0 ? "- " : "";

    cout << indent << branch << nodeKindToString(node->kind);
    if (!node->value.empty())
        cout << "  \"" << node->value << "\"";
    if (node->line > 0)
        cout << "  (line " << node->line << ")";
    cout << "\n";

    for (const auto& child : node->children)
        printParseTree(child, depth + 1);
}

// ============================================================
// §0  Infrastructure
// ============================================================

Parser::Parser(const vector<Token>& tokens)
    : tokens_(tokens), pos_(0) {}

shared_ptr<ASTNode> Parser::parse() { return parseTranslationUnit(); }

// ── Token-stream helpers ─────────────────────────────────────

Token Parser::currentToken() const {
    if (pos_ >= (int)tokens_.size()) return tokens_.back();
    return tokens_[pos_];
}

Token Parser::peekToken(int offset) const {
    int idx = pos_ + offset;
    if (idx >= (int)tokens_.size()) return tokens_.back();
    return tokens_[idx];
}

void Parser::advance() {
    if (pos_ < (int)tokens_.size() - 1) ++pos_;
}

bool Parser::match(TokenType t) const {
    return currentToken().getType() == t;
}

Token Parser::consume(TokenType t, const string& msg) {
    if (!match(t)) {
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": " + msg + " (got '" + currentToken().getLexeme() + "')");
    }
    Token tok = currentToken();
    advance();
    return tok;
}

bool Parser::isAtEnd() const { return match(EOF_TOK); }

void Parser::synchronize() {
    while (!isAtEnd()) {
        if (match(DELIM_SEMICOLON)) { advance(); return; }
        if (match(DELIM_RBRACE))    { return; }
        switch (currentToken().getType()) {
            case KW_IF:    case KW_WHILE:   case KW_FOR:      case KW_DO:
            case KW_RETURN: case KW_BREAK:  case KW_CONTINUE: case KW_GOTO:
            case KW_INT:   case KW_FLOAT:   case KW_DOUBLE:   case KW_CHAR:
            case KW_VOID:  case KW_LONG:    case KW_SHORT:    case KW_STRUCT:
            case KW_UNION: case KW_ENUM:    case KW_TYPEDEF:
            case KW_STATIC: case KW_EXTERN: case KW_CONST:    case KW_INLINE:
                return;
            default:
                advance();
        }
    }
}

const vector<ParseError>& Parser::getParseErrors() const {
    return parseErrors_;
}

// ── AST factory ─────────────────────────────────────────────

shared_ptr<ASTNode> Parser::node(NodeKind k, const string& v) const {
    return make_shared<ASTNode>(k, v, currentToken().getLine());
}

// ── Classification predicates ────────────────────────────────

bool Parser::isStorageClassSpecifier() const {
    switch (currentToken().getType()) {
        case TokenType::KW_TYPEDEF:      case TokenType::KW_EXTERN:
        case TokenType::KW_STATIC:       case TokenType::KW_THREAD_LOCAL:
        case TokenType::KW_AUTO:         case TokenType::KW_REGISTER:
            return true;
        default: return false;
    }
}

bool Parser::isTypeSpecifier() const {
    switch (currentToken().getType()) {
        case TokenType::KW_VOID:    case TokenType::KW_CHAR:
        case TokenType::KW_SHORT:   case TokenType::KW_INT:
        case TokenType::KW_LONG:    case TokenType::KW_FLOAT:
        case TokenType::KW_DOUBLE:  case TokenType::KW_SIGNED:
        case TokenType::KW_UNSIGNED:case TokenType::KW_BOOL:
        case TokenType::KW_COMPLEX: case TokenType::KW_STRUCT:
        case TokenType::KW_UNION:   case TokenType::KW_ENUM:
            return true;
        // _Atomic is a type-specifier only when followed by '('
        case TokenType::KW_ATOMIC:
            return peekToken().getType() == DELIM_LPAREN;
        // typedef-name
        case TokenType::IDENTIFIER:
            return typedefNames_.count(currentToken().getLexeme()) > 0;
        default: return false;
    }
}

bool Parser::isTypeQualifier() const {
    switch (currentToken().getType()) {
        case TokenType::KW_CONST:    case TokenType::KW_RESTRICT:
        case TokenType::KW_VOLATILE:
            return true;
        // _Atomic without '(' is a qualifier
        case TokenType::KW_ATOMIC:
            return peekToken().getType() != DELIM_LPAREN;
        default: return false;
    }
}

bool Parser::isFunctionSpecifier() const {
    return match(TokenType::KW_INLINE) || match(TokenType::KW_NORETURN);
}

bool Parser::isAlignmentSpecifier() const {
    return match(TokenType::KW_ALIGNAS);
}

bool Parser::isDeclarationSpecifier() const {
    return isStorageClassSpecifier() || isTypeSpecifier() ||
           isTypeQualifier()         || isFunctionSpecifier() ||
           isAlignmentSpecifier();
}

bool Parser::isAssignmentOperator() const {
    switch (currentToken().getType()) {
        case OP_ASSIGN:     case OP_PLUS_ASSIGN:
        case OP_MINUS_ASSIGN:   case OP_STAR_ASSIGN:
        case OP_SLASH_ASSIGN:   case OP_MOD_ASSIGN:
        case OP_AND_ASSIGN:     case OP_OR_ASSIGN:
        case OP_XOR_ASSIGN:   case OP_LSHIFT_ASSIGN:
        case OP_RSHIFT_ASSIGN:
            return true;
        default: return false;
    }
}

bool Parser::isUnaryOperator() const {
    switch (currentToken().getType()) {
        case OP_AMPERSAND:   case OP_STAR:
        case OP_PLUS:  case OP_MINUS:
        case OP_TILDE: case OP_NOT:
            return true;
        default: return false;
    }
}

// True when current token is '(' and the next token starts a type-name.
// Used to distinguish cast / compound-literal from grouped expression.
bool Parser::parenStartsTypeName() const {
    if (!match(TokenType::DELIM_LPAREN)) return false;
    switch (peekToken().getType()) {
        // type-specifier keywords
        case TokenType::KW_VOID:    case TokenType::KW_CHAR:
        case TokenType::KW_SHORT:   case TokenType::KW_INT:
        case TokenType::KW_LONG:    case TokenType::KW_FLOAT:
        case TokenType::KW_DOUBLE:  case TokenType::KW_SIGNED:
        case TokenType::KW_UNSIGNED:case TokenType::KW_BOOL:
        case TokenType::KW_COMPLEX: case TokenType::KW_STRUCT:
        case TokenType::KW_UNION:   case TokenType::KW_ENUM:
        // type-qualifier keywords
        case TokenType::KW_CONST:   case TokenType::KW_RESTRICT:
        case TokenType::KW_VOLATILE:case TokenType::KW_ATOMIC:
            return true;
        // typedef-name
        case TokenType::IDENTIFIER:
            return typedefNames_.count(peekToken().getLexeme()) > 0;
        default: return false;
    }
}

// ── Typedef bookkeeping ──────────────────────────────────────

bool Parser::specsAreTypedef(const shared_ptr<ASTNode>& specs) const {
    if (!specs) return false;
    for (auto& c : specs->children)
        if (c->kind == NodeKind::StorageClassSpecifier && c->value == "typedef")
            return true;
    return false;
}

string Parser::extractDeclName(const shared_ptr<ASTNode>& n) const {
    if (!n) return "";
    if (n->kind == NodeKind::Identifier) return n->value;
    for (auto& c : n->children) {
        string name = extractDeclName(c);
        if (!name.empty()) return name;
    }
    return "";
}

void Parser::registerTypedefName(const shared_ptr<ASTNode>& n) {
    string name = extractDeclName(n);
    if (!name.empty()) typedefNames_.insert(name);
}

// ============================================================
// §6  External Definitions
// ============================================================
// translation-unit:
//   external-declaration translation-unit'
// translation-unit':
//   external-declaration translation-unit' | ε

shared_ptr<ASTNode> Parser::parseTranslationUnit() {
    auto root = node(NodeKind::TranslationUnit);
    while (!isAtEnd()) {
        try {
            root->addChild(parseExternalDeclaration());
        } catch (const runtime_error& e) {
            parseErrors_.push_back({currentToken().getLine(), e.what()});
            synchronize();
        }
    }
    return root;
}

// external-declaration:
//   function-definition | declaration
//
// Both start with declaration-specifiers.  After parsing
// specifiers + first declarator we disambiguate:
//   '{' or K&R-decl-list  → function-definition
//   ';' / ',' / '='       → declaration

shared_ptr<ASTNode> Parser::parseExternalDeclaration() {
    // _Static_assert at file scope
    if (match(TokenType::KW_STATIC_ASSERT))
        return parseStaticAssertDeclaration();

    auto specs = parseDeclarationSpecifiers();
    bool isTypedef = specsAreTypedef(specs);

    // Bare "struct Foo;" style declaration
    if (match(DELIM_SEMICOLON)) {
        auto decl = node(NodeKind::Declaration);
        decl->addChild(specs);
        advance();
        return decl;
    }

    auto decl = parseDeclarator();
    if (isTypedef) registerTypedefName(decl);

    // Function definition?
    if (match(DELIM_LBRACE) || isDeclarationSpecifier())
        return parseFunctionDefinition(specs, decl);

    // Declaration — handle first init-declarator then rest of list
    auto fdNode = node(NodeKind::Declaration);
    fdNode->addChild(specs);

    auto initDeclList = node(NodeKind::InitDeclaratorList);
    auto firstInitDecl = node(NodeKind::InitDeclarator);
    firstInitDecl->addChild(decl);
    if (match(OP_ASSIGN)) {
        advance();
        firstInitDecl->addChild(parseInitializer());
    }
    initDeclList->addChild(firstInitDecl);

    while (match(DELIM_COMMA)) {
        advance();
        auto id = parseInitDeclarator(isTypedef);
        initDeclList->addChild(id);
    }
    fdNode->addChild(initDeclList);
    consume(DELIM_SEMICOLON, "expected ';' after declaration");
    return fdNode;
}

// function-definition:
//   declaration-specifiers declarator declaration-list(opt) compound-statement
shared_ptr<ASTNode> Parser::parseFunctionDefinition(
        shared_ptr<ASTNode> specs, shared_ptr<ASTNode> decl) {
    auto fn = node(NodeKind::FunctionDefinition);
    fn->addChild(specs);
    fn->addChild(decl);
    // K&R-style parameter declaration list
    if (isDeclarationSpecifier())
        fn->addChild(parseDeclarationList());
    fn->addChild(parseCompoundStatement());
    return fn;
}

// declaration-list:  declaration declaration-list' | ε
shared_ptr<ASTNode> Parser::parseDeclarationList() {
    auto dl = node(NodeKind::DeclarationList);
    while (isDeclarationSpecifier())
        dl->addChild(parseDeclaration());
    return dl;
}

// ============================================================
// §2  Declarations
// ============================================================
// declaration:
//   declaration-specifiers init-declarator-list(opt) ;
//   static_assert-declaration

shared_ptr<ASTNode> Parser::parseDeclaration() {
    if (match(TokenType::KW_STATIC_ASSERT))
        return parseStaticAssertDeclaration();

    auto specs = parseDeclarationSpecifiers();
    bool isTypedef = specsAreTypedef(specs);
    auto decl = node(NodeKind::Declaration);
    decl->addChild(specs);

    if (!match(DELIM_SEMICOLON))
        decl->addChild(parseInitDeclaratorList(isTypedef));

    consume(DELIM_SEMICOLON, "expected ';' after declaration");
    return decl;
}

// declaration-specifiers:
//   (storage-class-specifier | type-specifier | type-qualifier |
//    function-specifier | alignment-specifier)+
shared_ptr<ASTNode> Parser::parseDeclarationSpecifiers() {
    auto specs = node(NodeKind::DeclarationSpecifiers);
    while (isDeclarationSpecifier()) {
        if (isStorageClassSpecifier())
            specs->addChild(parseStorageClassSpecifier());
        else if (match(TokenType::KW_ATOMIC) &&
                 peekToken().getType() == DELIM_LPAREN)
            specs->addChild(parseAtomicTypeSpecifier());
        else if (isTypeSpecifier())
            specs->addChild(parseTypeSpecifier());
        else if (isTypeQualifier())
            specs->addChild(parseTypeQualifier());
        else if (isFunctionSpecifier())
            specs->addChild(parseFunctionSpecifier());
        else
            specs->addChild(parseAlignmentSpecifier());
    }
    if (specs->children.empty())
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": expected declaration-specifier");
    return specs;
}

shared_ptr<ASTNode> Parser::parseStorageClassSpecifier() {
    Token tok = currentToken();
    advance();
    return node(NodeKind::StorageClassSpecifier, tok.getLexeme());
}

shared_ptr<ASTNode> Parser::parseTypeSpecifier() {
    // struct / union / enum get their own sub-parsers
    if (match(TokenType::KW_STRUCT) || match(TokenType::KW_UNION))
        return parseStructOrUnionSpecifier();
    if (match(TokenType::KW_ENUM))
        return parseEnumSpecifier();
    if (match(TokenType::KW_ATOMIC))          // _Atomic(type-name)
        return parseAtomicTypeSpecifier();
    // Simple keyword or typedef-name
    Token tok = currentToken();
    advance();
    return node(NodeKind::TypeSpecifier, tok.getLexeme());
}

shared_ptr<ASTNode> Parser::parseAtomicTypeSpecifier() {
    auto n = node(NodeKind::AtomicTypeSpecifier, "_Atomic");
    consume(TokenType::KW_ATOMIC,  "expected '_Atomic'");
    consume(DELIM_LPAREN,     "expected '(' after '_Atomic'");
    n->addChild(parseTypeName());
    consume(DELIM_RPAREN,     "expected ')' after type-name in _Atomic");
    return n;
}

shared_ptr<ASTNode> Parser::parseTypeQualifier() {
    Token tok = currentToken();
    advance();
    return node(NodeKind::TypeQualifier, tok.getLexeme());
}

shared_ptr<ASTNode> Parser::parseFunctionSpecifier() {
    Token tok = currentToken();
    advance();
    return node(NodeKind::FunctionSpecifier, tok.getLexeme());
}

shared_ptr<ASTNode> Parser::parseAlignmentSpecifier() {
    auto n = node(NodeKind::AlignmentSpecifier);
    consume(TokenType::KW_ALIGNAS, "expected '_Alignas'");
    consume(DELIM_LPAREN,     "expected '(' after '_Alignas'");
    // Either a type-name or a constant-expression
    if (isDeclarationSpecifier())
        n->addChild(parseTypeName());
    else
        n->addChild(parseConstantExpression());
    consume(DELIM_RPAREN, "expected ')' after _Alignas argument");
    return n;
}

// ── Struct / union ───────────────────────────────────────────
// struct-or-union-specifier:
//   struct-or-union identifier(opt) { struct-declaration-list }
//   struct-or-union identifier

shared_ptr<ASTNode> Parser::parseStructOrUnionSpecifier() {
    auto n = node(NodeKind::StructOrUnionSpecifier,
                  currentToken().getLexeme());   // "struct" or "union"
    advance();

    if (match(TokenType::IDENTIFIER)) {
        n->addChild(node(NodeKind::Identifier, currentToken().getLexeme()));
        advance();
    }

    if (match(DELIM_LBRACE)) {
        advance();
        n->addChild(parseStructDeclarationList());
        consume(DELIM_RBRACE, "expected '}' after struct body");
    } else if (n->children.empty()) {
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": struct/union requires a tag or body");
    }
    return n;
}

// struct-declaration-list: struct-declaration+
shared_ptr<ASTNode> Parser::parseStructDeclarationList() {
    auto list = node(NodeKind::StructDeclarationList);
    while (!match(DELIM_RBRACE) && !isAtEnd())
        list->addChild(parseStructDeclaration());
    return list;
}

// struct-declaration:
//   specifier-qualifier-list struct-declarator-list ;
//   static_assert-declaration
shared_ptr<ASTNode> Parser::parseStructDeclaration() {
    if (match(TokenType::KW_STATIC_ASSERT))
        return parseStaticAssertDeclaration();

    auto sd = node(NodeKind::StructDeclaration);
    sd->addChild(parseSpecifierQualifierList());
    if (!match(DELIM_SEMICOLON))
        sd->addChild(parseStructDeclaratorList());
    consume(DELIM_SEMICOLON, "expected ';' after struct member");
    return sd;
}

// specifier-qualifier-list: (type-specifier | type-qualifier)+
shared_ptr<ASTNode> Parser::parseSpecifierQualifierList() {
    auto sql = node(NodeKind::SpecifierQualifierList);
    while (isTypeSpecifier() || isTypeQualifier()) {
        if (match(TokenType::KW_ATOMIC) &&
            peekToken().getType() == DELIM_LPAREN)
            sql->addChild(parseAtomicTypeSpecifier());
        else if (isTypeSpecifier())
            sql->addChild(parseTypeSpecifier());
        else
            sql->addChild(parseTypeQualifier());
    }
    if (sql->children.empty())
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": expected specifier-qualifier");
    return sql;
}

// struct-declarator-list: struct-declarator (, struct-declarator)*
shared_ptr<ASTNode> Parser::parseStructDeclaratorList() {
    auto list = node(NodeKind::StructDeclaratorList);
    list->addChild(parseStructDeclarator());
    while (match(DELIM_COMMA)) {
        advance();
        list->addChild(parseStructDeclarator());
    }
    return list;
}

// struct-declarator:
//   declarator
//   declarator(opt) : constant-expression
shared_ptr<ASTNode> Parser::parseStructDeclarator() {
    auto sd = node(NodeKind::StructDeclarator);
    if (!match(DELIM_COLON))
        sd->addChild(parseDeclarator());
    if (match(DELIM_COLON)) {
        advance();
        sd->addChild(parseConstantExpression());
    }
    return sd;
}

// ── Enum ─────────────────────────────────────────────────────
// enum-specifier:
//   enum identifier(opt) { enumerator-list enum-body-tail }
//   enum identifier
// enum-body-tail: , | ε

shared_ptr<ASTNode> Parser::parseEnumSpecifier() {
    auto n = node(NodeKind::EnumSpecifier);
    consume(TokenType::KW_ENUM, "expected 'enum'");

    if (match(TokenType::IDENTIFIER)) {
        n->addChild(node(NodeKind::Identifier, currentToken().getLexeme()));
        advance();
    }

    if (match(DELIM_LBRACE)) {
        advance();
        n->addChild(parseEnumeratorList());
        if (match(DELIM_COMMA)) advance();  // optional trailing ','
        consume(DELIM_RBRACE, "expected '}' after enumerator-list");
    } else if (n->children.empty()) {
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": enum requires a tag or body");
    }
    return n;
}

// enumerator-list: enumerator (, enumerator)*
shared_ptr<ASTNode> Parser::parseEnumeratorList() {
    auto list = node(NodeKind::EnumeratorList);
    list->addChild(parseEnumerator());
    while (match(DELIM_COMMA) &&
           peekToken().getType() != DELIM_RBRACE) {
        advance();
        list->addChild(parseEnumerator());
    }
    return list;
}

// enumerator:
//   enumeration-constant
//   enumeration-constant = constant-expression
shared_ptr<ASTNode> Parser::parseEnumerator() {
    auto e = node(NodeKind::Enumerator);
    e->addChild(node(NodeKind::Identifier,
                     consume(TokenType::IDENTIFIER,
                             "expected enumerator name").getLexeme()));
    if (match(OP_ASSIGN)) {
        advance();
        e->addChild(parseConstantExpression());
    }
    return e;
}

// ── Init-declarator list ─────────────────────────────────────
// init-declarator-list: init-declarator (, init-declarator)*
shared_ptr<ASTNode> Parser::parseInitDeclaratorList(bool isTypedef) {
    auto list = node(NodeKind::InitDeclaratorList);
    list->addChild(parseInitDeclarator(isTypedef));
    while (match(DELIM_COMMA)) {
        advance();
        list->addChild(parseInitDeclarator(isTypedef));
    }
    return list;
}

// init-declarator: declarator | declarator = initializer
shared_ptr<ASTNode> Parser::parseInitDeclarator(bool isTypedef) {
    auto id = node(NodeKind::InitDeclarator);
    auto decl = parseDeclarator();
    if (isTypedef) registerTypedefName(decl);
    id->addChild(decl);
    if (match(OP_ASSIGN)) {
        advance();
        id->addChild(parseInitializer());
    }
    return id;
}

// _Static_assert ( constant-expression , string-literal ) ;
shared_ptr<ASTNode> Parser::parseStaticAssertDeclaration() {
    auto n = node(NodeKind::StaticAssertDeclaration);
    consume(TokenType::KW_STATIC_ASSERT, "expected '_Static_assert'");
    consume(DELIM_LPAREN,           "expected '('");
    n->addChild(parseConstantExpression());
    consume(DELIM_COMMA,            "expected ','");
    n->addChild(node(NodeKind::StringLiteral,
                     consume(LIT_STRING,
                             "expected string-literal").getLexeme()));
    consume(DELIM_RPAREN,    "expected ')'");
    consume(DELIM_SEMICOLON, "expected ';'");
    return n;
}

// ============================================================
// §3  Declarators
// ============================================================
// declarator: pointer(opt) direct-declarator
shared_ptr<ASTNode> Parser::parseDeclarator() {
    auto d = node(NodeKind::Declarator);
    if (match(TokenType::OP_STAR)) d->addChild(parsePointer());
    d->addChild(parseDirectDeclarator());
    return d;
}

// direct-declarator:
//   identifier direct-declarator'
//   ( declarator ) direct-declarator'
shared_ptr<ASTNode> Parser::parseDirectDeclarator() {
    shared_ptr<ASTNode> base;

    if (match(TokenType::IDENTIFIER)) {
        base = node(NodeKind::Identifier, currentToken().getLexeme());
        advance();
    } else if (match(TokenType::DELIM_LPAREN)) {
        advance();
        base = parseDeclarator();
        consume(DELIM_RPAREN, "expected ')' in declarator");
    } else {
        throw runtime_error(
            "Parse error at line " + to_string(currentToken().getLine()) +
            ": expected identifier or '(' in declarator");
    }
    return parseDirectDeclaratorTail(base);
}

// direct-declarator':
//   [ dd-bracket-suffix ] direct-declarator'
//   ( dd-paren-suffix )   direct-declarator'
//   ε
shared_ptr<ASTNode> Parser::parseDirectDeclaratorTail(shared_ptr<ASTNode> base) {
    while (match(DELIM_LBRACKET) || match(TokenType::DELIM_LPAREN)) {
        auto dd = node(NodeKind::DirectDeclarator);
        dd->addChild(base);
        if (match(DELIM_LBRACKET)) {
            advance();
            dd->addChild(parseDdBracketSuffix());
            consume(DELIM_RBRACKET, "expected ']'");
        } else {
            advance();  // '('
            dd->addChild(parseDdParenSuffix());
            consume(DELIM_RPAREN, "expected ')'");
        }
        base = dd;
    }
    return base;
}

// dd-bracket-suffix:
//   type-qualifier-list(opt) assignment-expression(opt)
//   static type-qualifier-list(opt) assignment-expression
//   type-qualifier-list static assignment-expression
//   type-qualifier-list(opt) *
shared_ptr<ASTNode> Parser::parseDdBracketSuffix() {
    auto n = node(NodeKind::DirectDeclarator, "[]");

    if (match(TokenType::KW_STATIC)) {
        n->addChild(node(NodeKind::StorageClassSpecifier, "static"));
        advance();
        if (isTypeQualifier()) n->addChild(parseTypeQualifierList());
        n->addChild(parseAssignmentExpression());
        return n;
    }

    if (isTypeQualifier()) {
        n->addChild(parseTypeQualifierList());
        if (match(TokenType::KW_STATIC)) {
            n->addChild(node(NodeKind::StorageClassSpecifier, "static"));
            advance();
            n->addChild(parseAssignmentExpression());
            return n;
        }
        if (match(TokenType::OP_STAR)) { advance(); return n; }
    }

    if (match(TokenType::OP_STAR)) { advance(); return n; }

    if (!match(DELIM_RBRACKET))
        n->addChild(parseAssignmentExpression());

    return n;
}

// dd-paren-suffix:
//   parameter-type-list
//   identifier-list(opt)
shared_ptr<ASTNode> Parser::parseDdParenSuffix() {
    if (match(DELIM_RPAREN))
        return node(NodeKind::ParameterList);  // empty

    // If the next token is a type-specifier/qualifier, it's a prototype
    if (isDeclarationSpecifier())
        return parseParameterTypeList();

    // Otherwise it could be an identifier-list (K&R) or empty
    if (match(TokenType::IDENTIFIER))
        return parseIdentifierList();

    return node(NodeKind::ParameterList);  // empty
}

// pointer: * type-qualifier-list(opt) pointer(opt)
shared_ptr<ASTNode> Parser::parsePointer() {
    auto p = node(NodeKind::Pointer);
    consume(OP_STAR, "expected '*' in pointer");
    if (isTypeQualifier()) p->addChild(parseTypeQualifierList());
    if (match(TokenType::OP_STAR)) p->addChild(parsePointer());
    return p;
}

// type-qualifier-list: type-qualifier+
shared_ptr<ASTNode> Parser::parseTypeQualifierList() {
    auto list = node(NodeKind::TypeQualifierList);
    while (isTypeQualifier()) list->addChild(parseTypeQualifier());
    return list;
}

// parameter-type-list:
//   parameter-list
//   parameter-list , ...
shared_ptr<ASTNode> Parser::parseParameterTypeList() {
    auto ptl = node(NodeKind::ParameterTypeList);
    ptl->addChild(parseParameterList());
    if (match(DELIM_COMMA) &&
        peekToken().getType() == DELIM_ELLIPSIS) {
        advance();  // ','
        advance();  // '...'
        ptl->addChild(node(NodeKind::Ellipsis, "..."));
    }
    return ptl;
}

// parameter-list: parameter-declaration (, parameter-declaration)*
shared_ptr<ASTNode> Parser::parseParameterList() {
    auto list = node(NodeKind::ParameterList);
    list->addChild(parseParameterDeclaration());
    while (match(DELIM_COMMA) &&
           peekToken().getType() != DELIM_ELLIPSIS) {
        advance();
        list->addChild(parseParameterDeclaration());
    }
    return list;
}

// parameter-declaration:
//   declaration-specifiers declarator
//   declaration-specifiers abstract-declarator(opt)
shared_ptr<ASTNode> Parser::parseParameterDeclaration() {
    auto pd = node(NodeKind::ParameterDeclaration);
    pd->addChild(parseDeclarationSpecifiers());
    if (match(TokenType::OP_STAR) || match(TokenType::IDENTIFIER) ||
        match(TokenType::DELIM_LPAREN)) {
        // Could be declarator or abstract-declarator.
        // Try declarator first (it is a strict superset here).
        pd->addChild(parseDeclarator());
    } else if (match(DELIM_LBRACKET)) {
        pd->addChild(parseAbstractDeclarator());
    }
    return pd;
}

// identifier-list: identifier (, identifier)*
shared_ptr<ASTNode> Parser::parseIdentifierList() {
    auto list = node(NodeKind::IdentifierList);
    list->addChild(node(NodeKind::Identifier,
                        consume(TokenType::IDENTIFIER,
                                "expected identifier").getLexeme()));
    while (match(DELIM_COMMA) &&
           peekToken().getType() == TokenType::IDENTIFIER) {
        advance();
        list->addChild(node(NodeKind::Identifier,
                            consume(TokenType::IDENTIFIER,
                                    "expected identifier").getLexeme()));
    }
    return list;
}

// type-name: specifier-qualifier-list abstract-declarator(opt)
shared_ptr<ASTNode> Parser::parseTypeName() {
    auto tn = node(NodeKind::TypeName);
    tn->addChild(parseSpecifierQualifierList());
    if (match(TokenType::OP_STAR) || match(TokenType::DELIM_LPAREN) ||
        match(DELIM_LBRACKET))
        tn->addChild(parseAbstractDeclarator());
    return tn;
}

// abstract-declarator:
//   pointer
//   pointer(opt) direct-abstract-declarator
shared_ptr<ASTNode> Parser::parseAbstractDeclarator() {
    auto ad = node(NodeKind::AbstractDeclarator);
    if (match(TokenType::OP_STAR)) ad->addChild(parsePointer());
    if (match(TokenType::DELIM_LPAREN) || match(DELIM_LBRACKET))
        ad->addChild(parseDirectAbstractDeclarator());
    return ad;
}

// direct-abstract-declarator:
//   ( dad-paren-suffix ) direct-abstract-declarator'
//   direct-abstract-declarator'
// direct-abstract-declarator':
//   [ dad-bracket-suffix ] direct-abstract-declarator'
//   ( parameter-type-list(opt) ) direct-abstract-declarator'
//   ε
shared_ptr<ASTNode> Parser::parseDirectAbstractDeclarator() {
    auto dad = node(NodeKind::DirectAbstractDeclarator);

    if (match(TokenType::DELIM_LPAREN)) {
        advance();
        dad->addChild(parseDadParenSuffix());
        consume(DELIM_RPAREN, "expected ')'");
    }

    // Tail repetitions
    while (match(DELIM_LBRACKET) || match(TokenType::DELIM_LPAREN)) {
        if (match(DELIM_LBRACKET)) {
            advance();
            dad->addChild(parseDadBracketSuffix());
            consume(DELIM_RBRACKET, "expected ']'");
        } else {
            advance();  // '('
            if (!match(DELIM_RPAREN))
                dad->addChild(parseParameterTypeList());
            consume(DELIM_RPAREN, "expected ')'");
        }
    }
    return dad;
}

// dad-paren-suffix: abstract-declarator | parameter-type-list(opt)
shared_ptr<ASTNode> Parser::parseDadParenSuffix() {
    if (match(TokenType::OP_STAR) || match(TokenType::DELIM_LPAREN) ||
        match(DELIM_LBRACKET))
        return parseAbstractDeclarator();
    if (isDeclarationSpecifier())
        return parseParameterTypeList();
    return node(NodeKind::Empty);
}

// dad-bracket-suffix:
//   type-qualifier-list(opt) assignment-expression(opt)
//   static type-qualifier-list(opt) assignment-expression
//   type-qualifier-list static assignment-expression
//   *
shared_ptr<ASTNode> Parser::parseDadBracketSuffix() {
    auto n = node(NodeKind::DirectAbstractDeclarator, "[]");
    if (match(TokenType::OP_STAR)) { advance(); return n; }

    if (match(TokenType::KW_STATIC)) {
        advance();
        if (isTypeQualifier()) n->addChild(parseTypeQualifierList());
        n->addChild(parseAssignmentExpression());
        return n;
    }
    if (isTypeQualifier()) {
        n->addChild(parseTypeQualifierList());
        if (match(TokenType::KW_STATIC)) {
            advance();
            n->addChild(parseAssignmentExpression());
            return n;
        }
    }
    if (!match(DELIM_RBRACKET))
        n->addChild(parseAssignmentExpression());
    return n;
}

// ============================================================
// §4  Initializers
// ============================================================
// initializer:
//   assignment-expression
//   { initializer-list }
//   { initializer-list , }
shared_ptr<ASTNode> Parser::parseInitializer() {
    auto n = node(NodeKind::Initializer);
    if (match(DELIM_LBRACE)) {
        advance();
        n->addChild(parseInitializerList());
        if (match(DELIM_COMMA)) advance();  // optional trailing ','
        consume(DELIM_RBRACE, "expected '}' after initializer-list");
    } else {
        n->addChild(parseAssignmentExpression());
    }
    return n;
}

// initializer-list:
//   designation(opt) initializer (, designation(opt) initializer)*
shared_ptr<ASTNode> Parser::parseInitializerList() {
    auto list = node(NodeKind::InitializerList);
    do {
        if (match(DELIM_COMMA)) advance();
        if (match(DELIM_RBRACE)) break;

        // designation?
        if (match(DELIM_LBRACKET) ||
            (match(OP_DOT) &&
             peekToken().getType() == TokenType::IDENTIFIER))
            list->addChild(parseDesignation());

        list->addChild(parseInitializer());
    } while (match(DELIM_COMMA) &&
             peekToken().getType() != DELIM_RBRACE);
    return list;
}

// designation: designator-list =
shared_ptr<ASTNode> Parser::parseDesignation() {
    auto d = node(NodeKind::Designation);
    d->addChild(parseDesignatorList());
    consume(OP_ASSIGN, "expected '=' after designator");
    return d;
}

// designator-list: designator+
shared_ptr<ASTNode> Parser::parseDesignatorList() {
    auto list = node(NodeKind::DesignatorList);
    while (match(DELIM_LBRACKET) || match(OP_DOT))
        list->addChild(parseDesignator());
    return list;
}

// designator:
//   [ constant-expression ]
//   . identifier
shared_ptr<ASTNode> Parser::parseDesignator() {
    auto d = node(NodeKind::Designator);
    if (match(DELIM_LBRACKET)) {
        advance();
        d->addChild(parseConstantExpression());
        consume(DELIM_RBRACKET, "expected ']' in designator");
    } else {
        consume(OP_DOT, "expected '.' in designator");
        d->addChild(node(NodeKind::Identifier,
                         consume(TokenType::IDENTIFIER,
                                 "expected identifier after '.'").getLexeme()));
    }
    return d;
}

// ============================================================
// §5  Statements
// ============================================================
// statement:
//   labeled-statement | compound-statement | expression-statement
//   selection-statement | iteration-statement | jump-statement
shared_ptr<ASTNode> Parser::parseStatement() {
    // labeled-statement: identifier ':' …  or  case/default
    if (match(TokenType::KW_CASE) || match(TokenType::KW_DEFAULT))
        return parseLabeledStatement();
    if (match(TokenType::IDENTIFIER) &&
        peekToken().getType() == DELIM_COLON)
        return parseLabeledStatement();

    if (match(DELIM_LBRACE))       return parseCompoundStatement();
    if (match(TokenType::KW_IF) ||
        match(TokenType::KW_SWITCH))    return parseSelectionStatement();
    if (match(TokenType::KW_WHILE) ||
        match(TokenType::KW_DO)    ||
        match(TokenType::KW_FOR))       return parseIterationStatement();
    if (match(TokenType::KW_GOTO)  ||
        match(TokenType::KW_CONTINUE) ||
        match(TokenType::KW_BREAK)   ||
        match(TokenType::KW_RETURN))    return parseJumpStatement();

    return parseExpressionStatement();
}

// labeled-statement:
//   identifier : statement
//   case constant-expression : statement
//   default : statement
shared_ptr<ASTNode> Parser::parseLabeledStatement() {
    auto ls = node(NodeKind::LabeledStatement);
    if (match(TokenType::KW_CASE)) {
        advance();
        ls->addChild(node(NodeKind::Identifier, "case"));
        ls->addChild(parseConstantExpression());
        consume(DELIM_COLON, "expected ':' after case expression");
    } else if (match(TokenType::KW_DEFAULT)) {
        advance();
        ls->addChild(node(NodeKind::Identifier, "default"));
        consume(DELIM_COLON, "expected ':' after default");
    } else {
        ls->addChild(node(NodeKind::Identifier, currentToken().getLexeme()));
        advance();
        consume(DELIM_COLON, "expected ':' after label");
    }
    ls->addChild(parseStatement());
    return ls;
}

// compound-statement: { block-item-list(opt) }
shared_ptr<ASTNode> Parser::parseCompoundStatement() {
    auto cs = node(NodeKind::CompoundStatement);
    consume(DELIM_LBRACE, "expected '{'");
    if (!match(DELIM_RBRACE))
        cs->addChild(parseBlockItemList());
    consume(DELIM_RBRACE, "expected '}'");
    return cs;
}

// block-item-list: block-item+
shared_ptr<ASTNode> Parser::parseBlockItemList() {
    auto list = node(NodeKind::BlockItemList);
    while (!match(DELIM_RBRACE) && !isAtEnd()) {
        try {
            list->addChild(parseBlockItem());
        } catch (const runtime_error& e) {
            parseErrors_.push_back({currentToken().getLine(), e.what()});
            synchronize();
        }
    }
    return list;
}

// block-item: declaration | statement
// A block item is a declaration if it starts with a declaration-specifier.
shared_ptr<ASTNode> Parser::parseBlockItem() {
    auto bi = node(NodeKind::BlockItem);
    if (isDeclarationSpecifier() || match(TokenType::KW_STATIC_ASSERT))
        bi->addChild(parseDeclaration());
    else
        bi->addChild(parseStatement());
    return bi;
}

// expression-statement: expression(opt) ;
shared_ptr<ASTNode> Parser::parseExpressionStatement() {
    auto es = node(NodeKind::ExpressionStatement);
    if (!match(DELIM_SEMICOLON))
        es->addChild(parseExpression());
    consume(DELIM_SEMICOLON, "expected ';'");
    return es;
}

// selection-statement:
//   if ( expression ) statement else-part
//   switch ( expression ) statement
// else-part: else statement | ε
shared_ptr<ASTNode> Parser::parseSelectionStatement() {
    auto ss = node(NodeKind::SelectionStatement,
                   currentToken().getLexeme());  // "if" or "switch"
    advance();
    consume(DELIM_LPAREN, "expected '('");
    ss->addChild(parseExpression());
    consume(DELIM_RPAREN, "expected ')'");
    ss->addChild(parseStatement());
    // else-part (greedy / innermost-match for dangling-else)
    if (ss->value == "if" && match(TokenType::KW_ELSE)) {
        advance();
        ss->addChild(parseStatement());
    }
    return ss;
}

// iteration-statement:
//   while ( expression ) statement
//   do statement while ( expression ) ;
//   for ( expression(opt) ; expression(opt) ; expression(opt) ) statement
//   for ( declaration expression(opt) ; expression(opt) ) statement
shared_ptr<ASTNode> Parser::parseIterationStatement() {
    auto is = node(NodeKind::IterationStatement,
                   currentToken().getLexeme());  // "while"/"do"/"for"
    if (match(TokenType::KW_WHILE)) {
        advance();
        consume(DELIM_LPAREN, "expected '('");
        is->addChild(parseExpression());
        consume(DELIM_RPAREN, "expected ')'");
        is->addChild(parseStatement());

    } else if (match(TokenType::KW_DO)) {
        advance();
        is->addChild(parseStatement());
        consume(TokenType::KW_WHILE, "expected 'while'");
        consume(DELIM_LPAREN,   "expected '('");
        is->addChild(parseExpression());
        consume(DELIM_RPAREN,   "expected ')'");
        consume(DELIM_SEMICOLON,"expected ';'");

    } else {  // for
        advance();
        consume(DELIM_LPAREN, "expected '('");
        // Init: declaration or expression(opt) ;
        if (isDeclarationSpecifier())
            is->addChild(parseDeclaration());  // consumes ';'
        else {
            auto init = node(NodeKind::ExpressionStatement);
            if (!match(DELIM_SEMICOLON))
                init->addChild(parseExpression());
            consume(DELIM_SEMICOLON, "expected ';' in for");
            is->addChild(init);
        }
        // Condition
        if (!match(DELIM_SEMICOLON)) is->addChild(parseExpression());
        consume(DELIM_SEMICOLON, "expected ';' in for");
        // Increment
        if (!match(DELIM_RPAREN))    is->addChild(parseExpression());
        consume(DELIM_RPAREN,  "expected ')'");
        is->addChild(parseStatement());
    }
    return is;
}

// jump-statement:
//   goto identifier ;
//   continue ;   break ;
//   return expression(opt) ;
shared_ptr<ASTNode> Parser::parseJumpStatement() {
    auto js = node(NodeKind::JumpStatement, currentToken().getLexeme());
    advance();
    if (js->value == "goto") {
        js->addChild(node(NodeKind::Identifier,
                          consume(TokenType::IDENTIFIER,
                                  "expected label after 'goto'").getLexeme()));
    } else if (js->value == "return") {
        if (!match(DELIM_SEMICOLON))
            js->addChild(parseExpression());
    }
    consume(DELIM_SEMICOLON, "expected ';' after jump statement");
    return js;
}

// ============================================================
// §1  Expressions
// ============================================================

// expression: assignment-expression (, assignment-expression)*
shared_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseAssignmentExpression();
    while (match(DELIM_COMMA)) {
        auto op = currentToken().getLexeme();
        advance();
        auto right = parseAssignmentExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// assignment-expression:
//   conditional-expression
//   unary-expression assignment-operator assignment-expression
//
// Because unary-expression is a prefix of conditional-expression we
// parse conditional-expression and, if an assignment operator follows,
// treat the result as the LHS (this covers all well-formed programs).
shared_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto lhs = parseConditionalExpression();
    if (isAssignmentOperator()) {
        auto op = currentToken().getLexeme();
        advance();
        auto rhs = parseAssignmentExpression();
        auto ae = node(NodeKind::AssignmentExpression, op);
        ae->addChild(lhs);
        ae->addChild(rhs);
        return ae;
    }
    return lhs;
}

// conditional-expression:
//   logical-OR-expression
//   logical-OR-expression ? expression : conditional-expression
shared_ptr<ASTNode> Parser::parseConditionalExpression() {
    auto cond = parseLogicalOrExpression();
    if (match(DELIM_QUESTION)) {
        advance();
        auto ce = node(NodeKind::ConditionalExpression, "?:");
        ce->addChild(cond);
        ce->addChild(parseExpression());
        consume(DELIM_COLON, "expected ':' in conditional expression");
        ce->addChild(parseConditionalExpression());
        return ce;
    }
    return cond;
}

// logical-OR-expression: logical-AND-expression (|| logical-AND-expression)*
shared_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    while (match(OP_OR)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseLogicalAndExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// logical-AND-expression: inclusive-OR-expression (&& inclusive-OR-expression)*
shared_ptr<ASTNode> Parser::parseLogicalAndExpression() {
    auto left = parseInclusiveOrExpression();
    while (match(OP_AND)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseInclusiveOrExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// inclusive-OR-expression: exclusive-OR-expression (| exclusive-OR-expression)*
shared_ptr<ASTNode> Parser::parseInclusiveOrExpression() {
    auto left = parseExclusiveOrExpression();
    while (match(OP_PIPE)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseExclusiveOrExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// exclusive-OR-expression: AND-expression (^ AND-expression)*
shared_ptr<ASTNode> Parser::parseExclusiveOrExpression() {
    auto left = parseAndExpression();
    while (match(OP_CARET)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseAndExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// AND-expression: equality-expression (& equality-expression)*
shared_ptr<ASTNode> Parser::parseAndExpression() {
    auto left = parseEqualityExpression();
    while (match(OP_AMPERSAND)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseEqualityExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// equality-expression: relational-expression ((==|!=) relational-expression)*
shared_ptr<ASTNode> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    while (match(OP_EQ) || match(OP_NEQ)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseRelationalExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// relational-expression: shift-expression ((<|>|<=|>=) shift-expression)*
shared_ptr<ASTNode> Parser::parseRelationalExpression() {
    auto left = parseShiftExpression();
    while (match(OP_LT)    || match(OP_GT) ||
           match(OP_LTE) || match(OP_GTE)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseShiftExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// shift-expression: additive-expression ((<<|>>) additive-expression)*
shared_ptr<ASTNode> Parser::parseShiftExpression() {
    auto left = parseAdditiveExpression();
    while (match(OP_LSHIFT) || match(OP_RSHIFT)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseAdditiveExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// additive-expression: multiplicative-expression ((+|-) multiplicative-expression)*
shared_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    while (match(OP_PLUS) || match(OP_MINUS)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseMultiplicativeExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// multiplicative-expression: cast-expression ((*|/|%) cast-expression)*
shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto left = parseCastExpression();
    while (match(TokenType::OP_STAR) || match(OP_SLASH) ||
           match(OP_PERCENT)) {
        auto op = currentToken().getLexeme(); advance();
        auto right = parseCastExpression();
        auto bin = node(NodeKind::BinaryExpression, op);
        bin->addChild(left); bin->addChild(right); left = bin;
    }
    return left;
}

// cast-expression:
//   unary-expression
//   ( type-name ) cast-expression          ← if NOT followed by '{'
//   ( type-name ) { initializer-list }     ← compound literal
shared_ptr<ASTNode> Parser::parseCastExpression() {
    if (parenStartsTypeName()) {
        advance();  // consume '('
        auto tn = parseTypeName();
        consume(DELIM_RPAREN, "expected ')' after type-name");

        if (match(DELIM_LBRACE)) {
            // Compound literal: ( type-name ) { initializer-list }
            advance();  // '{'
            auto cl = node(NodeKind::CompoundLiteral);
            cl->addChild(tn);
            cl->addChild(parseInitializerList());
            if (match(DELIM_COMMA)) advance();  // trailing ','
            consume(DELIM_RBRACE, "expected '}'");
            return parsePostfixTail(cl);
        }

        // Cast expression
        auto ce = node(NodeKind::CastExpression);
        ce->addChild(tn);
        ce->addChild(parseCastExpression());
        return ce;
    }
    return parseUnaryExpression();
}

// unary-expression:
//   postfix-expression
//   ++ unary-expression
//   -- unary-expression
//   unary-operator cast-expression   (unary-operator: & * + - ~ !)
//   sizeof unary-expression
//   sizeof ( type-name )
//   _Alignof ( type-name )
shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (match(OP_PLUSPLUS) || match(OP_MINUSMINUS)) {
        auto op = currentToken().getLexeme(); advance();
        auto ue = node(NodeKind::UnaryExpression, op);
        ue->addChild(parseUnaryExpression());
        return ue;
    }
    if (isUnaryOperator()) {
        auto op = currentToken().getLexeme(); advance();
        auto ue = node(NodeKind::UnaryExpression, op);
        ue->addChild(parseCastExpression());
        return ue;
    }
    if (match(TokenType::KW_SIZEOF)) {
        advance();
        auto ue = node(NodeKind::UnaryExpression, "sizeof");
        if (parenStartsTypeName()) {
            advance();  // '('
            ue->addChild(parseTypeName());
            consume(DELIM_RPAREN, "expected ')' after sizeof type-name");
        } else {
            ue->addChild(parseUnaryExpression());
        }
        return ue;
    }
    if (match(TokenType::KW_ALIGNOF)) {
        advance();
        auto ue = node(NodeKind::UnaryExpression, "_Alignof");
        consume(DELIM_LPAREN, "expected '(' after _Alignof");
        ue->addChild(parseTypeName());
        consume(DELIM_RPAREN, "expected ')' after _Alignof type-name");
        return ue;
    }
    return parsePostfixExpression();
}

// postfix-expression:
//   primary-expression postfix'
//   ( type-name ) { initializer-list } postfix'   ← handled in parseCastExpression
// postfix':
//   [ expression ] postfix'
//   ( argument-expression-list(opt) ) postfix'
//   . identifier postfix'
//   -> identifier postfix'
//   ++ postfix'
//   -- postfix'
//   ε
shared_ptr<ASTNode> Parser::parsePostfixExpression() {
    return parsePostfixTail(parsePrimaryExpression());
}

shared_ptr<ASTNode> Parser::parsePostfixTail(shared_ptr<ASTNode> base) {
    while (true) {
        if (match(DELIM_LBRACKET)) {
            advance();
            auto pf = node(NodeKind::PostfixExpression, "[]");
            pf->addChild(base);
            pf->addChild(parseExpression());
            consume(DELIM_RBRACKET, "expected ']'");
            base = pf;

        } else if (match(TokenType::DELIM_LPAREN)) {
            advance();
            auto pf = node(NodeKind::PostfixExpression, "()");
            pf->addChild(base);
            if (!match(DELIM_RPAREN))
                pf->addChild(parseArgumentExpressionList());
            consume(DELIM_RPAREN, "expected ')'");
            base = pf;

        } else if (match(OP_DOT)) {
            advance();
            auto pf = node(NodeKind::PostfixExpression, ".");
            pf->addChild(base);
            pf->addChild(node(NodeKind::Identifier,
                              consume(TokenType::IDENTIFIER,
                                      "expected identifier after '.'").getLexeme()));
            base = pf;

        } else if (match(OP_ARROW)) {
            advance();
            auto pf = node(NodeKind::PostfixExpression, "->");
            pf->addChild(base);
            pf->addChild(node(NodeKind::Identifier,
                              consume(TokenType::IDENTIFIER,
                                      "expected identifier after '->'").getLexeme()));
            base = pf;

        } else if (match(OP_PLUSPLUS) || match(OP_MINUSMINUS)) {
            auto op = currentToken().getLexeme(); advance();
            auto pf = node(NodeKind::PostfixExpression, op + "(post)");
            pf->addChild(base);
            base = pf;

        } else {
            break;
        }
    }
    return base;
}

// primary-expression:
//   identifier
//   constant
//   string-literal
//   ( expression )
//   generic-selection
shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    auto pe = node(NodeKind::PrimaryExpression);

    if (match(TokenType::IDENTIFIER)) {
        pe->addChild(node(NodeKind::Identifier, currentToken().getLexeme()));
        advance();
        return pe;
    }
    if (match(LIT_INT) || match(LIT_FLOAT) ||
        match(LIT_CHAR)) {
        pe->addChild(node(NodeKind::Constant, currentToken().getLexeme()));
        advance();
        return pe;
    }
    if (match(LIT_STRING)) {
        pe->addChild(node(NodeKind::StringLiteral, currentToken().getLexeme()));
        advance();
        return pe;
    }
    if (match(TokenType::KW_GENERIC)) {
        pe->addChild(parseGenericSelection());
        return pe;
    }
    if (match(TokenType::DELIM_LPAREN)) {
        advance();
        pe->addChild(parseExpression());
        consume(DELIM_RPAREN, "expected ')' after expression");
        return pe;
    }
    throw runtime_error(
        "Parse error at line " + to_string(currentToken().getLine()) +
        ": unexpected token '" + currentToken().getLexeme() +
        "' in primary expression");
}

// argument-expression-list: assignment-expression (, assignment-expression)*
shared_ptr<ASTNode> Parser::parseArgumentExpressionList() {
    auto list = node(NodeKind::ArgumentExpressionList);
    list->addChild(parseAssignmentExpression());
    while (match(DELIM_COMMA)) {
        advance();
        list->addChild(parseAssignmentExpression());
    }
    return list;
}

// generic-selection:
//   _Generic ( assignment-expression , generic-assoc-list )
shared_ptr<ASTNode> Parser::parseGenericSelection() {
    auto gs = node(NodeKind::GenericSelection);
    consume(TokenType::KW_GENERIC, "expected '_Generic'");
    consume(DELIM_LPAREN,     "expected '('");
    gs->addChild(parseAssignmentExpression());
    consume(DELIM_COMMA,      "expected ','");
    gs->addChild(parseGenericAssocList());
    consume(DELIM_RPAREN,     "expected ')'");
    return gs;
}

// generic-assoc-list: generic-association (, generic-association)*
shared_ptr<ASTNode> Parser::parseGenericAssocList() {
    auto list = node(NodeKind::GenericAssocList);
    list->addChild(parseGenericAssociation());
    while (match(DELIM_COMMA)) {
        advance();
        list->addChild(parseGenericAssociation());
    }
    return list;
}

// generic-association:
//   type-name : assignment-expression
//   default   : assignment-expression
shared_ptr<ASTNode> Parser::parseGenericAssociation() {
    auto ga = node(NodeKind::GenericAssociation);
    if (match(TokenType::KW_DEFAULT)) {
        ga->addChild(node(NodeKind::Identifier, "default"));
        advance();
    } else {
        ga->addChild(parseTypeName());
    }
    consume(DELIM_COLON, "expected ':' in generic association");
    ga->addChild(parseAssignmentExpression());
    return ga;
}

// constant-expression: conditional-expression
// (semantically constrained; syntactically identical)
shared_ptr<ASTNode> Parser::parseConstantExpression() {
    return parseConditionalExpression();
}