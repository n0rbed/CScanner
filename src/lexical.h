#ifndef LEXICAL_H
#define LEXICAL_H
#include "scanner.h"
#include "symbol_table.h"

class Lexical
{
    SymbolTable symTable;
    Scanner scanner;
public:
    Lexical(const string& filename);
    ~Lexical();
    void Run();
    SymbolTable getSymbolTable() const;
    string getSourceCode() const;
    void editSourceCode();
    Scanner& getScanner();

};

#endif // LEXICAL_H
