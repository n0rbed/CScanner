#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <iostream>
// #include <qobject.h>
#include <sstream>
using namespace std;

struct SymbolRow {
    int rowNumber;
    string id;
};

class SymbolTable {
private:
    vector<SymbolRow> rows;

public:

    int insert(const string& id);

    bool contains(const string& id) const;

    int getRowNumber(const string& id) const;

    const vector<SymbolRow>& getRows() const;

    string print() const;

};

#endif