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

void printAST(const shared_ptr<ASTNode>& node, int depth) {
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
        printAST(child, depth + 1);
}