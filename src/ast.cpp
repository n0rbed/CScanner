#include "ast.h"
#include <iostream>

string nodeKindToString(NodeKind k) {
    switch (k) {
        case NodeKind::TranslationUnit:          return "TranslationUnit";
        case NodeKind::FunctionDefinition:       return "FunctionDefinition";
        case NodeKind::Declaration:              return "Declaration";
        case NodeKind::DeclarationList:          return "DeclarationList";
        case NodeKind::DeclarationSpecifiers:    return "DeclarationSpecifiers";
        case NodeKind::StorageClassSpecifier:    return "StorageClassSpecifier";
        case NodeKind::TypeSpecifier:            return "TypeSpecifier";
        case NodeKind::TypeQualifier:            return "TypeQualifier";
        case NodeKind::FunctionSpecifier:        return "FunctionSpecifier";
        case NodeKind::AlignmentSpecifier:       return "AlignmentSpecifier";
        case NodeKind::AtomicTypeSpecifier:      return "AtomicTypeSpecifier";
        case NodeKind::StructOrUnionSpecifier:   return "StructOrUnionSpecifier";
        case NodeKind::StructDeclarationList:    return "StructDeclarationList";
        case NodeKind::StructDeclaration:        return "StructDeclaration";
        case NodeKind::SpecifierQualifierList:   return "SpecifierQualifierList";
        case NodeKind::StructDeclaratorList:     return "StructDeclaratorList";
        case NodeKind::StructDeclarator:         return "StructDeclarator";
        case NodeKind::EnumSpecifier:            return "EnumSpecifier";
        case NodeKind::EnumeratorList:           return "EnumeratorList";
        case NodeKind::Enumerator:               return "Enumerator";
        case NodeKind::InitDeclaratorList:       return "InitDeclaratorList";
        case NodeKind::InitDeclarator:           return "InitDeclarator";
        case NodeKind::Declarator:               return "Declarator";
        case NodeKind::DirectDeclarator:         return "DirectDeclarator";
        case NodeKind::Pointer:                  return "Pointer";
        case NodeKind::TypeQualifierList:        return "TypeQualifierList";
        case NodeKind::ParameterTypeList:        return "ParameterTypeList";
        case NodeKind::ParameterList:            return "ParameterList";
        case NodeKind::ParameterDeclaration:     return "ParameterDeclaration";
        case NodeKind::IdentifierList:           return "IdentifierList";
        case NodeKind::TypeName:                 return "TypeName";
        case NodeKind::AbstractDeclarator:       return "AbstractDeclarator";
        case NodeKind::DirectAbstractDeclarator: return "DirectAbstractDeclarator";
        case NodeKind::Initializer:              return "Initializer";
        case NodeKind::InitializerList:          return "InitializerList";
        case NodeKind::Designation:              return "Designation";
        case NodeKind::DesignatorList:           return "DesignatorList";
        case NodeKind::Designator:               return "Designator";
        case NodeKind::CompoundStatement:        return "CompoundStatement";
        case NodeKind::BlockItemList:            return "BlockItemList";
        case NodeKind::BlockItem:                return "BlockItem";
        case NodeKind::ExpressionStatement:      return "ExpressionStatement";
        case NodeKind::LabeledStatement:         return "LabeledStatement";
        case NodeKind::SelectionStatement:       return "SelectionStatement";
        case NodeKind::IterationStatement:       return "IterationStatement";
        case NodeKind::JumpStatement:            return "JumpStatement";
        case NodeKind::GenericSelection:         return "GenericSelection";
        case NodeKind::GenericAssocList:         return "GenericAssocList";
        case NodeKind::GenericAssociation:       return "GenericAssociation";
        case NodeKind::Expression:               return "Expression";
        case NodeKind::AssignmentExpression:     return "AssignmentExpression";
        case NodeKind::ConditionalExpression:    return "ConditionalExpression";
        case NodeKind::BinaryExpression:         return "BinaryExpression";
        case NodeKind::UnaryExpression:          return "UnaryExpression";
        case NodeKind::CastExpression:           return "CastExpression";
        case NodeKind::PostfixExpression:        return "PostfixExpression";
        case NodeKind::CompoundLiteral:          return "CompoundLiteral";
        case NodeKind::ArgumentExpressionList:   return "ArgumentExpressionList";
        case NodeKind::PrimaryExpression:        return "PrimaryExpression";
        case NodeKind::Identifier:               return "Identifier";
        case NodeKind::Constant:                 return "Constant";
        case NodeKind::StringLiteral:            return "StringLiteral";
        case NodeKind::StaticAssertDeclaration:  return "StaticAssertDeclaration";
        case NodeKind::Ellipsis:                 return "Ellipsis";
        case NodeKind::Empty:                    return "Empty";
        default:                                 return "Unknown";
    }
}

// ── AST collapsing helpers ───────────────────────────────────────────────────

// Nodes that are completely transparent: they are skipped entirely and their
// children are printed at the *same* depth as the node itself would have been.
// These are pure grammar list/wrapper rules with no semantic content of their
// own (the wrapper adds nothing that isn't already conveyed by context).
static bool isTransparent(NodeKind k) {
    switch (k) {
        case NodeKind::BlockItemList:       // grammar list — children are the items
        case NodeKind::BlockItem:           // always exactly one child
        case NodeKind::PrimaryExpression:   // always exactly one child (the real expr)
        case NodeKind::ExpressionStatement: // the ';' is implicit; child is the expr
            return true;
        default:
            return false;
    }
}

// Nodes that act as single-child pass-throughs: when they have exactly one
// child they add no information, so we skip straight to the child.  When they
// have multiple children they ARE meaningful and are kept.
static bool collapseWhenSingle(NodeKind k) {
    switch (k) {
        case NodeKind::DeclarationSpecifiers: // e.g. just "int" — skip the wrapper
        case NodeKind::InitDeclaratorList:    // comma list; single item adds nothing
        case NodeKind::InitDeclarator:        // declarator with no initializer
        case NodeKind::Declarator:            // pointer-less declarator
        case NodeKind::Initializer:           // scalar initializer
            return true;
        default:
            return false;
    }
}

// ── printAST ─────────────────────────────────────────────────────────────────
// Prints a simplified AST by eliding pure grammar-artifact nodes.
// Compare with printParseTree (in parser.cpp) which dumps every node verbatim.

void printAST(const shared_ptr<ASTNode>& node, int depth) {
    if (!node) return;

    // 1. Completely transparent nodes: skip the node, recurse into children at
    //    the *same* depth so they appear at the parent's indentation level.
    if (isTransparent(node->kind)) {
        for (const auto& child : node->children)
            printAST(child, depth);
        return;
    }

    // 2. Single-child pass-through nodes: skip the wrapper and go straight to
    //    the child (avoids chains like Declarator → Identifier "x").
    if (collapseWhenSingle(node->kind) && node->children.size() == 1) {
        printAST(node->children[0], depth);
        return;
    }

    // 3. Everything else: print this node, then recurse into its children.
    string indent(depth * 2, ' ');
    string branch = depth > 0 ? "- " : "";

    cout << indent << branch << nodeKindToString(node->kind);
    if (!node->value.empty())
        cout << "  \"" << node->value << "\"";
    if (node->line > 0)
        cout << "  (line " << node->line << ")";
    cout << "\n";

    for (const auto& child : node->children)
        printAST(child, depth + 1);
}