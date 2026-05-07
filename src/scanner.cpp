#include "scanner.h"
#include "token.h"
#include <cctype>
#include <stdexcept>
using namespace std;

// ── initializeKeywords ────────────────────────────────────────────────────
void Scanner::initializeKeywords() {
    keywords["auto"]     = KW_AUTO;
    keywords["break"]    = KW_BREAK;
    keywords["case"]     = KW_CASE;
    keywords["char"]     = KW_CHAR;
    keywords["const"]    = KW_CONST;
    keywords["continue"] = KW_CONTINUE;
    keywords["default"]  = KW_DEFAULT;
    keywords["do"]       = KW_DO;
    keywords["double"]   = KW_DOUBLE;
    keywords["else"]     = KW_ELSE;
    keywords["enum"]     = KW_ENUM;
    keywords["extern"]   = KW_EXTERN;
    keywords["float"]    = KW_FLOAT;
    keywords["for"]      = KW_FOR;
    keywords["goto"]     = KW_GOTO;
    keywords["if"]       = KW_IF;
    keywords["inline"]   = KW_INLINE;
    keywords["int"]      = KW_INT;
    keywords["long"]     = KW_LONG;
    keywords["register"] = KW_REGISTER;
    keywords["restrict"] = KW_RESTRICT;
    keywords["return"]   = KW_RETURN;
    keywords["short"]    = KW_SHORT;
    keywords["signed"]   = KW_SIGNED;
    keywords["sizeof"]   = KW_SIZEOF;
    keywords["static"]   = KW_STATIC;
    keywords["struct"]   = KW_STRUCT;
    keywords["switch"]   = KW_SWITCH;
    keywords["typedef"]  = KW_TYPEDEF;
    keywords["union"]    = KW_UNION;
    keywords["unsigned"] = KW_UNSIGNED;
    keywords["void"]     = KW_VOID;
    keywords["volatile"] = KW_VOLATILE;
    keywords["while"]    = KW_WHILE;
}

Scanner::Scanner(const string& filename) : lineNumber(1), position(0) {
    sourceFile.open(filename);
    if (!sourceFile.is_open())
        throw runtime_error("Could not open file: " + filename);

    initializeKeywords();

    // read entire file into sourceString for easier scanning
    sourceString.assign((istreambuf_iterator<char>(sourceFile)),
                        istreambuf_iterator<char>());
    sourceFile.close();
}

Scanner::~Scanner() {
    if (sourceFile.is_open())
        sourceFile.close();
}


string Scanner::getSourceString() const{
    return sourceString;
}


// Helpers
char Scanner::getChar() {
    if (position < (int)sourceString.size())
        return sourceString[position];
    return EOF;
}

char Scanner::peekChar() {
    return peekAhead(1);
}

char Scanner::peekAhead(int offset) {
    int idx = position + offset;
    if (idx < (int)sourceString.size())
        return sourceString[idx];
    return EOF;
}

void Scanner::advance() {
    if (position < (int)sourceString.size()) {
        if (sourceString[position] == '\n')
            lineNumber++;
        position++;
    }
}

void Scanner::skipWhitespace() {
    while (isspace((unsigned char)getChar()))
        advance();
}

// Handles  // ... \n - You should only call this if you already know
// the current char is '/' and next char is '/'
void Scanner::skipLineComment() {
    while (getChar() != '\n' && getChar() != (char)EOF)
        advance();
}

// Handles  /* ... */
// Call condition same as above
void Scanner::skipBlockComment() {
    advance(); advance();   // consume '/' and '*'
    while (getChar() != (char)EOF) {
        if (getChar() == '*' && peekChar() == '/') {
            advance(); advance();   // consume '*' and '/'
            return;
        }
        advance();
    }
    throw runtime_error("Unterminated block comment starting near line " +
                        to_string(lineNumber));
}

// This loop occurs in scanNumber a lot, so we factor it out.
string Scanner::getDigitSerial(string num) {
    while (isdigit((unsigned char)getChar())) {
        num += getChar();
        advance();
    }
    return num;
}

// Handles integer suffixes: u, l, ul, ull, etc.
string Scanner::getIntegerSuffix(string num) {
    for (int i = 0; i < 2; i++) {
        if (getChar() == 'u' || getChar() == 'U' ||
            getChar() == 'l' || getChar() == 'L') {
            num += getChar();
            advance();
        }
        else {
            break;
        }
    }
    return num;
}

// Handles:  decimal (42), hex (0xFF), octal (0755), float (3.14, 1.0e-5f)
Token Scanner::scanNumber() {
    string num;
    bool isFloat = false;

    // Hex literal
    if (getChar() == '0' && (peekChar() == 'x' || peekChar() == 'X')) {
        num += getChar(); advance();   // '0'
        num += getChar(); advance();   // 'x'/'X'
        while (isxdigit((unsigned char)getChar())) {
            num += getChar();
            advance();
        }
        // optional suffix: u, l, ul, etc.
        num = getIntegerSuffix(num);
        return Token(LIT_INT, num, lineNumber);
    }

    // Decimal / octal / float
    num = getDigitSerial(num);

    // Fractional part e.g. 153.21 (153 is captured by the loop above)
    if (getChar() == '.' && isdigit((unsigned char)peekChar())) {
        isFloat = true;
        num += getChar(); advance();   // '.'
        num = getDigitSerial(num);
    }

    // Exponent part: e/E followed by optional +/- and digits
    if (getChar() == 'e' || getChar() == 'E') {
        isFloat = true;
        num += getChar(); advance();
        if (getChar() == '+' || getChar() == '-') {
            num += getChar(); advance();
        }
        num = getDigitSerial(num);
    }

    // Float suffix: f / F / l / L
    if (isFloat && (getChar() == 'f' || getChar() == 'F' ||
                    getChar() == 'l' || getChar() == 'L')) {
        num += getChar();
        advance();
    }

    // Integer suffixes: u, l, ul, ull, etc.
    if (!isFloat) {
        num = getIntegerSuffix(num);
    }

    return Token(isFloat ? LIT_FLOAT : LIT_INT, num, lineNumber);
}

// Identifiers start with a letter or '_', followed by letters, digits, '_'
Token Scanner::scanIdentifierOrKeyword() {
    string id;
    while (isalnum((unsigned char)getChar()) || getChar() == '_') {
        id += getChar();
        advance();
    }
    auto it = keywords.find(id);
    if (it != keywords.end())
        return Token(it->second, id, lineNumber);
    return Token(IDENTIFIER, id, lineNumber);
}

// Captures strings like "..." including escape sequences
Token Scanner::scanStringLiteral() {
    string str;
    str += getChar(); advance();   // opening '"'

    while (getChar() != '"' && getChar() != (char)EOF) {
        if (getChar() == '\\') {   // escape sequence: keep both chars
            str += getChar(); advance();
            if (getChar() != (char)EOF) {
                str += getChar(); advance();
            }
        } else {
            str += getChar(); advance();
        }
    }

    if (getChar() == '"') {
        str += getChar(); advance();   // closing '"'
    } else {
        throw runtime_error("Unterminated string literal at line " +
                            to_string(lineNumber));
    }

    return Token(LIT_STRING, str, lineNumber);
}

Token Scanner::scanCharLiteral() {
    string ch;
    ch += getChar(); advance();   // opening '\''

    while (getChar() != '\'' && getChar() != (char)EOF) {
        if (getChar() == '\\') {
            ch += getChar(); advance();
            if (getChar() != (char)EOF) {
                ch += getChar(); advance();
            }
        } else {
            ch += getChar(); advance();
        }
    }

    if (getChar() == '\'') {
        ch += getChar(); advance();   // closing '\''
    } else {
        throw runtime_error("Unterminated char literal at line " +
                            to_string(lineNumber));
    }

    return Token(LIT_CHAR, ch, lineNumber);
}

// preprocessor directives are: #include, #define, etc.
// Reads from '#' to the end of the logical line (handles \ continuations)
Token Scanner::scanPreprocessorDirective() {
    string directive;
    while (getChar() != '\n' && getChar() != (char)EOF) {
        if (getChar() == '\\' && peekChar() == '\n') {
            // line continuation inside a macro: consume both and keep reading
            advance(); advance();
        } else {
            directive += getChar();
            advance();
        }
    }
    return Token(PP_DIRECTIVE, directive, lineNumber);
}

// Uses a greedy longest-match strategy with peekChar() / peekAhead()
Token Scanner::scanOperatorOrDelimiter() {
    char c  = getChar();
    char c2 = peekChar();
    char c3 = peekAhead(2);

    // Helper lambda to consume n chars and return a token
    auto make = [&](int len, TokenType t, const string& lex) -> Token {
        for (int i = 0; i < len; i++) advance();
        return Token(t, lex, lineNumber);
    };

    switch (c) {
    // ── 3-char operators ──────────────────────────────────────────────
    case '<':
        if (c2 == '<' && c3 == '=') return make(3, OP_LSHIFT_ASSIGN, "<<=");
        if (c2 == '<')              return make(2, OP_LSHIFT, "<<");
        if (c2 == '=')              return make(2, OP_LTE,    "<=");
        return make(1, OP_LT, "<");

    case '>':
        if (c2 == '>' && c3 == '=') return make(3, OP_RSHIFT_ASSIGN, ">>=");
        if (c2 == '>')              return make(2, OP_RSHIFT, ">>");
        if (c2 == '=')              return make(2, OP_GTE,    ">=");
        return make(1, OP_GT, ">");

    case '.':
        if (c2 == '.' && c3 == '.') return make(3, DELIM_ELLIPSIS, "...");
        return make(1, OP_DOT, ".");

    // ── 2-char operators ──────────────────────────────────────────────
    case '+':
        if (c2 == '+') return make(2, OP_PLUSPLUS,    "++");
        if (c2 == '=') return make(2, OP_PLUS_ASSIGN, "+=");
        return make(1, OP_PLUS, "+");

    case '-':
        if (c2 == '-') return make(2, OP_MINUSMINUS,  "--");
        if (c2 == '>') return make(2, OP_ARROW,        "->");
        if (c2 == '=') return make(2, OP_MINUS_ASSIGN, "-=");
        return make(1, OP_MINUS, "-");

    case '*':
        if (c2 == '=') return make(2, OP_STAR_ASSIGN,  "*=");
        return make(1, OP_STAR, "*");

    case '/':
        if (c2 == '=') return make(2, OP_SLASH_ASSIGN, "/=");
        return make(1, OP_SLASH, "/");   // '/' alone (comments handled above)

    case '%':
        if (c2 == '=') return make(2, OP_MOD_ASSIGN, "%=");
        return make(1, OP_PERCENT, "%");

    case '=':
        if (c2 == '=') return make(2, OP_EQ,     "==");
        return make(1, OP_ASSIGN, "=");

    case '!':
        if (c2 == '=') return make(2, OP_NEQ, "!=");
        return make(1, OP_NOT, "!");

    case '&':
        if (c2 == '&') return make(2, OP_AND,        "&&");
        if (c2 == '=') return make(2, OP_AND_ASSIGN, "&=");
        return make(1, OP_AMPERSAND, "&");

    case '|':
        if (c2 == '|') return make(2, OP_OR,        "||");
        if (c2 == '=') return make(2, OP_OR_ASSIGN, "|=");
        return make(1, OP_PIPE, "|");

    case '^':
        if (c2 == '=') return make(2, OP_XOR_ASSIGN, "^=");
        return make(1, OP_CARET, "^");

    // ── Single-char ────────────────────────────────────────────────────
    case '~': return make(1, OP_TILDE,        "~");
    case '(': return make(1, DELIM_LPAREN,    "(");
    case ')': return make(1, DELIM_RPAREN,    ")");
    case '{': return make(1, DELIM_LBRACE,    "{");
    case '}': return make(1, DELIM_RBRACE,    "}");
    case '[': return make(1, DELIM_LBRACKET,  "[");
    case ']': return make(1, DELIM_RBRACKET,  "]");
    case ';': return make(1, DELIM_SEMICOLON, ";");
    case ',': return make(1, DELIM_COMMA,     ",");
    case ':': return make(1, DELIM_COLON,     ":");
    case '?': return make(1, DELIM_QUESTION,  "?");

    default:
        advance();
        return Token(UNKNOWN, string(1, c), lineNumber);
    }
}

// ── getTokens ─────────────────────────────────────────────────────────────
vector<Token> Scanner::getTokens() {
    while (position < (int)sourceString.size()) {
        skipWhitespace();

        char c  = getChar();
        char c2 = peekChar();

        if (c == (char)EOF) {
            break;
        }

        // Line comment
        else if (c == '/' && c2 == '/') {
            skipLineComment();
        }

        // Block comment
        else if (c == '/' && c2 == '*') {
            skipBlockComment();
        }

        // Preprocessor directive
        else if (c == '#') {
            tokens.push_back(scanPreprocessorDirective());
        }

        // String literal
        else if (c == '"') {
            tokens.push_back(scanStringLiteral());
        }

        // Char literal
        else if (c == '\'') {
            tokens.push_back(scanCharLiteral());
        }

        // Number:  starts with a digit, or  .5  style float
        else if (isdigit((unsigned char)c) ||
                 (c == '.' && isdigit((unsigned char)c2))) {
            tokens.push_back(scanNumber());
        }

        // Identifier or keyword
        else if (isalpha((unsigned char)c) || c == '_') {
            tokens.push_back(scanIdentifierOrKeyword());
        }

        // Operator or delimiter
        else if (ispunct((unsigned char)c)) {
            tokens.push_back(scanOperatorOrDelimiter());
        }

        else {
            throw runtime_error(string("Unexpected character: '") + c +
                                "' at line " + to_string(lineNumber));
        }
    }
    tokens.push_back(Token(EOF_TOK, "", 0));
    return tokens;
}
