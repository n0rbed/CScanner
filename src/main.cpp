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
    Parser parser(tokens);
    shared_ptr<ASTNode> ast;
    try {
        ast = parser.parse();
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    // ── 3. Report errors ─────────────────────────────────────
    for (const auto& err : lexical.getScanner().getLexErrors())
        cerr << "Lexical error: line " << err.line << ", col " << err.col
             << ": " << err.message << "\n";
    for (const auto& err : parser.getParseErrors())
        cerr << "Syntax error: line " << err.line << ": " << err.message << "\n";
    bool anyErrors = !lexical.getScanner().getLexErrors().empty() ||
                     !parser.getParseErrors().empty();
    cout << (anyErrors ? "Compilation failed.\n\n" : "No errors.\n\n");

    if (!anyErrors)
    {
        // ── 4. Print parse tree ───────────────────────────────────
        cout << "=== Parse Tree ===\n";
        if (ast) printParseTree(ast);
        cout << "\n";

        // ── 5. Print AST ─────────────────────────────────────────
        cout << "=== AST ===\n";
        if (ast) printAST(ast);
    }

    return anyErrors ? 1 : 0;
}