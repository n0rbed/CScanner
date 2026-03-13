#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

class Scanner {
private:
    ifstream      sourceFile;
    string        sourceString;
    int           lineNumber;
    int           position;
    vector<Token> tokens;
    map<string, TokenType> keywords;

    void  initializeKeywords();

    char  getChar();
    char  peekChar();
    char  peekAhead(int offset);   // look further than one ahead
    void  advance();
    void  skipWhitespace();
    void  skipLineComment();       // //
    void  skipBlockComment();      // /* */

    Token scanNumber();
    Token scanIdentifierOrKeyword();
    Token scanStringLiteral();
    Token scanCharLiteral();
    Token scanPreprocessorDirective();
    Token scanOperatorOrDelimiter();

public:
    Scanner(const string& filename);
    ~Scanner();
    vector<Token> getTokens();
};

#endif // SCANNER_H
