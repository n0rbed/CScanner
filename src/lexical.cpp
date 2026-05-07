#include "lexical.h"

Lexical::Lexical(const string& filename) : scanner(filename) {


}

Scanner& Lexical::getScanner() {
    return scanner;
}

void Lexical::Run()
{
    try {
        // Create scanner
        //Scanner scanner("E:\\Programming Folder\\Qt Designs\\CompilerGUI\\sourceCode.c");
        vector<Token> tokens = scanner.getTokens();

        // Create symbol table


        // Print tokens + fill symbol table
        for (const auto &token: tokens) {
            cout << token << endl;

            // Only store identifiers
            if (token.getType() == IDENTIFIER) {
                int row = symTable.insert(token.getLexeme());
                cout << " -> inserted in row: " << row << endl;
            }

        }
        cout << symTable.print();
    }
    catch(const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

}

Lexical::~Lexical(){

}
SymbolTable Lexical::getSymbolTable() const{
    return symTable;
}






