#include "lexical.h"
#include "parser.h"
#include "ast.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // ── 0. Get source file path ──────────────────────────────
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <path-to-source-file>\n";
        cerr << "Example: " << argv[0] << " D:\\yassin\\test.c\n";
        return 1;
    }

    const string sourceFile = argv[1];

    // ── 1. Lex ───────────────────────────────────────────────
    cout << "=== Lexing: " << sourceFile << " ===\n";
    Lexical lexical(sourceFile);
    lexical.Run();

    vector<Token> tokens = lexical.getScanner().getTokens();
    cout << "Tokens produced: " << tokens.size() << "\n\n";

    // ── 2. Parse ─────────────────────────────────────────────
    cout << "=== Parsing ===\n";
    shared_ptr<ASTNode> ast;
    try {
        Parser parser(tokens);
        ast = parser.parse();
        cout << "Parse successful.\n\n";
    } catch (const exception& e) {
        cerr << "Parse error: " << e.what() << "\n";
        return 1;
    }

    // ── 3. Print AST ─────────────────────────────────────────
    cout << "=== AST ===\n";
    printAST(ast);

    return 0;
}