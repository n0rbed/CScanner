#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>
using namespace std;

enum TokenType {
    // ── Keywords ──────────────────────────────────────────────────────────
    KW_AUTO,     KW_BREAK,    KW_CASE,     KW_CHAR,     KW_CONST,
    KW_CONTINUE, KW_DEFAULT,  KW_DO,       KW_DOUBLE,   KW_ELSE,
    KW_ENUM,     KW_EXTERN,   KW_FLOAT,    KW_FOR,      KW_GOTO,
    KW_IF,       KW_INLINE,   KW_INT,      KW_LONG,     KW_REGISTER,
    KW_RESTRICT, KW_RETURN,   KW_SHORT,    KW_SIGNED,   KW_SIZEOF,
    KW_STATIC,   KW_STRUCT,   KW_SWITCH,   KW_TYPEDEF,  KW_UNION,
    KW_UNSIGNED, KW_VOID,     KW_VOLATILE, KW_WHILE,

    // ── Arithmetic operators ───────────────────────────────────────────────
    OP_PLUS,        // +
    OP_MINUS,       // -
    OP_STAR,        // *
    OP_SLASH,       // /
    OP_PERCENT,     // %
    OP_PLUSPLUS,    // ++
    OP_MINUSMINUS,  // --

    // ── Relational operators ───────────────────────────────────────────────
    OP_EQ,          // ==
    OP_NEQ,         // !=
    OP_LT,          // <
    OP_GT,          // >
    OP_LTE,         // <=
    OP_GTE,         // >=

    // ── Logical operators ──────────────────────────────────────────────────
    OP_AND,         // &&
    OP_OR,          // ||
    OP_NOT,         // !

    // ── Bitwise operators ──────────────────────────────────────────────────
    OP_AMPERSAND,   // &
    OP_PIPE,        // |
    OP_CARET,       // ^
    OP_TILDE,       // ~
    OP_LSHIFT,      // <<
    OP_RSHIFT,      // >>

    // ── Assignment operators ───────────────────────────────────────────────
    OP_ASSIGN,      // =
    OP_PLUS_ASSIGN, // +=
    OP_MINUS_ASSIGN,// -=
    OP_STAR_ASSIGN, // *=
    OP_SLASH_ASSIGN,// /=
    OP_MOD_ASSIGN,  // %=
    OP_AND_ASSIGN,  // &=
    OP_OR_ASSIGN,   // |=
    OP_XOR_ASSIGN,  // ^=
    OP_LSHIFT_ASSIGN,// <<=
    OP_RSHIFT_ASSIGN,// >>=

    // ── Member / pointer operators ─────────────────────────────────────────
    OP_DOT,         // .
    OP_ARROW,       // ->

    // ── Delimiters ─────────────────────────────────────────────────────────
    DELIM_LPAREN,   // (
    DELIM_RPAREN,   // )
    DELIM_LBRACE,   // {
    DELIM_RBRACE,   // }
    DELIM_LBRACKET, // [
    DELIM_RBRACKET, // ]
    DELIM_SEMICOLON,// ;
    DELIM_COMMA,    // ,
    DELIM_COLON,    // :
    DELIM_QUESTION, // ?
    DELIM_ELLIPSIS, // ...

    // ── Preprocessor ──────────────────────────────────────────────────────
    PP_DIRECTIVE,   // any line starting with #

    // ── Literals ──────────────────────────────────────────────────────────
    LIT_INT,        // e.g.  42, 0xFF, 0755
    LIT_FLOAT,      // e.g.  3.14, 1.0e-5
    LIT_CHAR,       // e.g.  'a', '\n'
    LIT_STRING,     // e.g.  "hello"

    // ── Other ─────────────────────────────────────────────────────────────
    IDENTIFIER,
    UNKNOWN
};

class Token {
private:
    TokenType type;
    string    lexeme;
    int       line;

    friend ostream& operator<<(ostream& os, const Token& t);

public:
    Token(TokenType type, const string& lexeme, int line);

    TokenType  getType()   const;
    string     getLexeme() const;
    int        getLine()   const;
    string     toString()  const;

    static string tokenTypeToString(TokenType type);
};

#endif // TOKEN_H
