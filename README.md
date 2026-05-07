# How do I run this?
Below is the command for compiling this project. Run it in the `src` directory of this project. This is without adding the GUI.
```
cl /std:c++17 /EHsc main.cpp ast.cpp parser.cpp scanner.cpp lexical.cpp token.cpp symbol_table.cpp /Fe:parser_test.exe
```
You can then run the output exe by doing the following:
```
.\parser_test.exe ..\tests_c\task3.c
```