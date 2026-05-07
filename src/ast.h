#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
using namespace std;

// ============================================================
//  NodeKind — one entry per distinct grammar construct.
//  Names mirror the grammar rule they represent.
// ============================================================
enum class NodeKind {
    // ── Top-level ─────────────────────────────────────────
    TranslationUnit,
    FunctionDefinition,
    Declaration,
    DeclarationList,        // K&R parameter declarations

    // ── Declaration specifiers ────────────────────────────
    DeclarationSpecifiers,
    StorageClassSpecifier,  // typedef / extern / static / …
    TypeSpecifier,          // int / char / struct … / typedef-name
    TypeQualifier,          // const / restrict / volatile / _Atomic
    FunctionSpecifier,      // inline / _Noreturn
    AlignmentSpecifier,     // _Alignas(…)
    AtomicTypeSpecifier,    // _Atomic(type-name)

    // ── Struct / union ────────────────────────────────────
    StructOrUnionSpecifier,
    StructDeclarationList,
    StructDeclaration,
    SpecifierQualifierList,
    StructDeclaratorList,
    StructDeclarator,

    // ── Enum ──────────────────────────────────────────────
    EnumSpecifier,
    EnumeratorList,
    Enumerator,

    // ── Init-declarator list ──────────────────────────────
    InitDeclaratorList,
    InitDeclarator,         // declarator  or  declarator = initializer

    // ── Declarators ───────────────────────────────────────
    Declarator,
    DirectDeclarator,
    Pointer,
    TypeQualifierList,
    ParameterTypeList,
    ParameterList,
    ParameterDeclaration,
    IdentifierList,
    TypeName,
    AbstractDeclarator,
    DirectAbstractDeclarator,

    // ── Initializers ──────────────────────────────────────
    Initializer,
    InitializerList,
    Designation,
    DesignatorList,
    Designator,

    // ── Statements ────────────────────────────────────────
    CompoundStatement,
    BlockItemList,
    BlockItem,
    ExpressionStatement,
    LabeledStatement,
    SelectionStatement,     // if / switch
    IterationStatement,     // while / do / for
    JumpStatement,          // goto / continue / break / return

    // ── Generic selection (C11) ───────────────────────────
    GenericSelection,
    GenericAssocList,
    GenericAssociation,

    // ── Expressions (high-level wrappers) ─────────────────
    Expression,
    AssignmentExpression,
    ConditionalExpression,
    BinaryExpression,       // value = operator string, e.g. "+"
    UnaryExpression,        // value = operator string
    CastExpression,
    PostfixExpression,      // value = operator, e.g. "[]", "()", ".", "->"
    CompoundLiteral,        // (type-name){ … }
    ArgumentExpressionList,
    PrimaryExpression,

    // ── Atomic terminals ──────────────────────────────────
    Identifier,
    Constant,
    StringLiteral,

    // ── Misc ──────────────────────────────────────────────
    StaticAssertDeclaration,
    Ellipsis,               // … in parameter-type-list
    Empty                   // ε / optional absent nodes
};

// ============================================================
//  ASTNode — generic tree node.
//
//  • kind    — which grammar construct this node represents
//  • value   — literal text for terminals (identifiers,
//               constants, operator strings)
//  • line    — source line for error reporting
//  • children — ordered list of child subtrees
// ============================================================
struct ASTNode {
    NodeKind               kind;
    string                 value;
    int                    line;
    vector<shared_ptr<ASTNode>> children;

    ASTNode(NodeKind k, const string& v = "", int l = 0)
        : kind(k), value(v), line(l) {}

    // Convenience: add a non-null child.
    void addChild(shared_ptr<ASTNode> child) {
        if (child) children.push_back(move(child));
    }
};

// ── Utilities ─────────────────────────────────────────────────────────────
string nodeKindToString(NodeKind k);
void   printAST(const shared_ptr<ASTNode>& node, int depth = 0);

#endif // AST_H