#include "scanner.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <source.c>" << endl;
        return 1;
    }

    try {
        Scanner scanner(argv[1]);
        vector<Token> tokens = scanner.getTokens();

        for (const auto& token : tokens)
            cout << token << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
