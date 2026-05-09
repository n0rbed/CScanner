#include "token.h"
#include <string>
#include <iostream>
using namespace std;

Token::Token(TokenType type, const string& lexeme, int line)
    : type(type), lexeme(lexeme), line(line) {}

TokenType Token::getType()   const { return type;   }
string    Token::getLexeme() const { return lexeme; }
int       Token::getLine()   const { return line;   }

string Token::tokenTypeToString(TokenType type) {
    switch (type) {
    // Keywords
    case KW_AUTO:      return "auto";
    case KW_BREAK:     return "break";
    case KW_CASE:      return "case";
    case KW_CHAR:      return "char";
    case KW_CONST:     return "const";
    case KW_CONTINUE:  return "continue";
    case KW_DEFAULT:   return "default";
    case KW_DO:        return "do";
    case KW_DOUBLE:    return "double";
    case KW_ELSE:      return "else";
    case KW_ENUM:      return "enum";
    case KW_EXTERN:    return "extern";
    case KW_FLOAT:     return "float";
    case KW_FOR:       return "for";
    case KW_GOTO:      return "goto";
    case KW_IF:        return "if";
    case KW_INLINE:    return "inline";
    case KW_INT:       return "int";
    case KW_LONG:      return "long";
    case KW_REGISTER:  return "register";
    case KW_RESTRICT:  return "restrict";
    case KW_RETURN:    return "return";
    case KW_SHORT:     return "short";
    case KW_SIGNED:    return "signed";
    case KW_SIZEOF:    return "sizeof";
    case KW_STATIC:    return "static";
    case KW_STRUCT:    return "struct";
    case KW_SWITCH:    return "switch";
    case KW_TYPEDEF:   return "typedef";
    case KW_UNION:     return "union";
    case KW_UNSIGNED:  return "unsigned";
    case KW_VOID:      return "void";
    case KW_VOLATILE:  return "volatile";
    case KW_WHILE:     return "while";

    // Arithmetic
    case OP_PLUS:          return "+";
    case OP_MINUS:         return "-";
    case OP_STAR:          return "*";
    case OP_SLASH:         return "/";
    case OP_PERCENT:       return "%";
    case OP_PLUSPLUS:      return "++";
    case OP_MINUSMINUS:    return "--";

    // Relational
    case OP_EQ:    return "==";
    case OP_NEQ:   return "!=";
    case OP_LT:    return "<";
    case OP_GT:    return ">";
    case OP_LTE:   return "<=";
    case OP_GTE:   return ">=";

    // Logical
    case OP_AND:   return "&&";
    case OP_OR:    return "||";
    case OP_NOT:   return "!";

    // Bitwise
    case OP_AMPERSAND: return "&";
    case OP_PIPE:      return "|";
    case OP_CARET:     return "^";
    case OP_TILDE:     return "~";
    case OP_LSHIFT:    return "<<";
    case OP_RSHIFT:    return ">>";

    // Assignment
    case OP_ASSIGN:         return "=";
    case OP_PLUS_ASSIGN:    return "+=";
    case OP_MINUS_ASSIGN:   return "-=";
    case OP_STAR_ASSIGN:    return "*=";
    case OP_SLASH_ASSIGN:   return "/=";
    case OP_MOD_ASSIGN:     return "%=";
    case OP_AND_ASSIGN:     return "&=";
    case OP_OR_ASSIGN:      return "|=";
    case OP_XOR_ASSIGN:     return "^=";
    case OP_LSHIFT_ASSIGN:  return "<<=";
    case OP_RSHIFT_ASSIGN:  return ">>=";

    // Member
    case OP_DOT:   return ".";
    case OP_ARROW: return "->";

    // Delimiters
    case DELIM_LPAREN:    return "(";
    case DELIM_RPAREN:    return ")";
    case DELIM_LBRACE:    return "{";
    case DELIM_RBRACE:    return "}";
    case DELIM_LBRACKET:  return "[";
    case DELIM_RBRACKET:  return "]";
    case DELIM_SEMICOLON: return ";";
    case DELIM_COMMA:     return ",";
    case DELIM_COLON:     return ":";
    case DELIM_QUESTION:  return "?";
    case DELIM_ELLIPSIS:  return "...";

    // Preprocessor
    case PP_DIRECTIVE: return "preprocessor-directive";

    // Literals
    case LIT_INT:    return "int-literal";
    case LIT_FLOAT:  return "float-literal";
    case LIT_CHAR:   return "char-literal";
    case LIT_STRING: return "string-literal";

    // Other
    case IDENTIFIER: return "identifier";
    case UNKNOWN:    return "unknown";
    case EOF_TOK:    return "EOF";
    default:         return "UNKNOWN_TOKEN_TYPE";
    }
}

string Token::toString() const {
    return "(" + tokenTypeToString(type) + ", \"" + lexeme + "\", line " + to_string(line) + ")";
}

ostream& operator<<(ostream& os, const Token& t) {
    os << t.toString();
    return os;
}