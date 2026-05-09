#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

struct LexError {
    int    line;
    int    col;
    string message;
};

class Scanner {
private:
    ifstream         sourceFile;
    string           sourceString;
    int              lineNumber;
    int              column_;
    int              position;
    vector<Token>    tokens;
    vector<LexError> lexErrors_;
    map<string, TokenType> keywords;

    void  initializeKeywords();

    char  getChar();
    char  peekChar();
    char  peekAhead(int offset);   // look further than one ahead
    void  advance();
    void  skipWhitespace();
    void  skipLineComment();       // //
    void  skipBlockComment();      // /* */

    string getDigitSerial(string num);
    string getIntegerSuffix(string num);
    Token scanNumber();

    Token scanIdentifierOrKeyword();
    Token scanStringLiteral();
    Token scanCharLiteral();
    Token scanPreprocessorDirective();
    Token scanOperatorOrDelimiter();

public:
    Scanner(const string& filename);
    ~Scanner();
    vector<Token>            getTokens();
    string                   getSourceString() const;
    const vector<LexError>&  getLexErrors()    const;
};

#endif // SCANNER_H
