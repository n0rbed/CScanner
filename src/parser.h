#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <unordered_set>
using namespace std;

// ============================================================
//  Parser — top-down recursive-descent parser for ISO C (C11).
//
//  Each private method corresponds to a grammar rule in the
//  ISO grammar document (all left-recursion already eliminated
//  and left-factored).  The public entry point is parse(),
//  which returns the root TranslationUnit node.
//
//  Typedef names are tracked in a symbol table so that the
//  parser can correctly resolve the typedef-name / identifier
//  ambiguity in declaration-specifiers.
// ============================================================
class Parser {
public:
    explicit Parser(const vector<Token>& tokens);
    shared_ptr<ASTNode> parse();   // entry point → TranslationUnit

private:
    // ── Token stream state ────────────────────────────────
    vector<Token> tokens_;
    int           pos_;

    // ── Typedef name table ────────────────────────────────
    // Populated as typedef declarations are parsed.
    unordered_set<string> typedefNames_;

    // ── Core token-stream helpers ─────────────────────────
    Token currentToken()           const;
    Token peekToken(int offset=1)  const;
    void  advance();
    bool  match(TokenType t)       const;
    Token consume(TokenType t, const string& msg);   // expect + advance
    bool  isAtEnd()                const;

    // ── Classification predicates ─────────────────────────
    bool isStorageClassSpecifier()  const;
    bool isTypeSpecifier()          const;
    bool isTypeQualifier()          const;
    bool isFunctionSpecifier()      const;
    bool isAlignmentSpecifier()     const;
    bool isDeclarationSpecifier()   const;
    bool isAssignmentOperator()     const;
    bool isUnaryOperator()          const;

    // Lookahead: does '(' at current pos hide a type-name?
    bool parenStartsTypeName()      const;

    // ── AST factory helper ────────────────────────────────
    shared_ptr<ASTNode> node(NodeKind k, const string& v = "") const;

    // ── Typedef bookkeeping ───────────────────────────────
    bool specsAreTypedef(const shared_ptr<ASTNode>& specs) const;
    void registerTypedefName(const shared_ptr<ASTNode>& declaratorOrInitDecl);
    string extractDeclName(const shared_ptr<ASTNode>& n) const;

    // ============================================================
    // §6  External definitions
    // ============================================================
    shared_ptr<ASTNode> parseTranslationUnit();
    shared_ptr<ASTNode> parseExternalDeclaration();
    shared_ptr<ASTNode> parseFunctionDefinition(shared_ptr<ASTNode> specs,
                                                 shared_ptr<ASTNode> decl);
    shared_ptr<ASTNode> parseDeclarationList();   // K&R param decls

    // ============================================================
    // §2  Declarations
    // ============================================================
    shared_ptr<ASTNode> parseDeclaration();
    shared_ptr<ASTNode> parseDeclarationSpecifiers();
    shared_ptr<ASTNode> parseStorageClassSpecifier();
    shared_ptr<ASTNode> parseTypeSpecifier();
    shared_ptr<ASTNode> parseTypeQualifier();
    shared_ptr<ASTNode> parseFunctionSpecifier();
    shared_ptr<ASTNode> parseAlignmentSpecifier();
    shared_ptr<ASTNode> parseAtomicTypeSpecifier();
    shared_ptr<ASTNode> parseStructOrUnionSpecifier();
    shared_ptr<ASTNode> parseStructDeclarationList();
    shared_ptr<ASTNode> parseStructDeclaration();
    shared_ptr<ASTNode> parseSpecifierQualifierList();
    shared_ptr<ASTNode> parseStructDeclaratorList();
    shared_ptr<ASTNode> parseStructDeclarator();
    shared_ptr<ASTNode> parseEnumSpecifier();
    shared_ptr<ASTNode> parseEnumeratorList();
    shared_ptr<ASTNode> parseEnumerator();
    shared_ptr<ASTNode> parseInitDeclaratorList(bool isTypedef);
    shared_ptr<ASTNode> parseInitDeclarator(bool isTypedef);
    shared_ptr<ASTNode> parseStaticAssertDeclaration();

    // ============================================================
    // §3  Declarators
    // ============================================================
    shared_ptr<ASTNode> parseDeclarator();
    shared_ptr<ASTNode> parseDirectDeclarator();
    shared_ptr<ASTNode> parseDirectDeclaratorTail(shared_ptr<ASTNode> base);
    shared_ptr<ASTNode> parseDdBracketSuffix();
    shared_ptr<ASTNode> parseDdParenSuffix();
    shared_ptr<ASTNode> parsePointer();
    shared_ptr<ASTNode> parseTypeQualifierList();
    shared_ptr<ASTNode> parseParameterTypeList();
    shared_ptr<ASTNode> parseParameterList();
    shared_ptr<ASTNode> parseParameterDeclaration();
    shared_ptr<ASTNode> parseIdentifierList();
    shared_ptr<ASTNode> parseTypeName();
    shared_ptr<ASTNode> parseAbstractDeclarator();
    shared_ptr<ASTNode> parseDirectAbstractDeclarator();
    shared_ptr<ASTNode> parseDadParenSuffix();
    shared_ptr<ASTNode> parseDadBracketSuffix();

    // ============================================================
    // §4  Initializers
    // ============================================================
    shared_ptr<ASTNode> parseInitializer();
    shared_ptr<ASTNode> parseInitializerList();
    shared_ptr<ASTNode> parseDesignation();
    shared_ptr<ASTNode> parseDesignatorList();
    shared_ptr<ASTNode> parseDesignator();

    // ============================================================
    // §5  Statements
    // ============================================================
    shared_ptr<ASTNode> parseStatement();
    shared_ptr<ASTNode> parseLabeledStatement();
    shared_ptr<ASTNode> parseCompoundStatement();
    shared_ptr<ASTNode> parseBlockItemList();
    shared_ptr<ASTNode> parseBlockItem();
    shared_ptr<ASTNode> parseExpressionStatement();
    shared_ptr<ASTNode> parseSelectionStatement();
    shared_ptr<ASTNode> parseIterationStatement();
    shared_ptr<ASTNode> parseJumpStatement();

    // ============================================================
    // §1  Expressions
    // ============================================================
    shared_ptr<ASTNode> parseExpression();
    shared_ptr<ASTNode> parseAssignmentExpression();
    shared_ptr<ASTNode> parseConditionalExpression();
    shared_ptr<ASTNode> parseLogicalOrExpression();
    shared_ptr<ASTNode> parseLogicalAndExpression();
    shared_ptr<ASTNode> parseInclusiveOrExpression();
    shared_ptr<ASTNode> parseExclusiveOrExpression();
    shared_ptr<ASTNode> parseAndExpression();
    shared_ptr<ASTNode> parseEqualityExpression();
    shared_ptr<ASTNode> parseRelationalExpression();
    shared_ptr<ASTNode> parseShiftExpression();
    shared_ptr<ASTNode> parseAdditiveExpression();
    shared_ptr<ASTNode> parseMultiplicativeExpression();
    shared_ptr<ASTNode> parseCastExpression();
    shared_ptr<ASTNode> parseUnaryExpression();
    shared_ptr<ASTNode> parsePostfixExpression();
    shared_ptr<ASTNode> parsePostfixTail(shared_ptr<ASTNode> base);
    shared_ptr<ASTNode> parsePrimaryExpression();
    shared_ptr<ASTNode> parseArgumentExpressionList();
    shared_ptr<ASTNode> parseGenericSelection();
    shared_ptr<ASTNode> parseGenericAssocList();
    shared_ptr<ASTNode> parseGenericAssociation();
    shared_ptr<ASTNode> parseConstantExpression();
};

#endif // PARSER_H
