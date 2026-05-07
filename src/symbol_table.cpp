#include "symbol_table.h"


// insert(id)
int SymbolTable::insert(const string& id) {

    for (const auto& row : rows) {
        if (row.id == id)
            return row.rowNumber;
    }

    SymbolRow newRow;
    newRow.rowNumber = rows.size() +1;
    newRow.id = id;

    rows.push_back(newRow);

    return newRow.rowNumber;
}

// contains(id)
bool SymbolTable::contains(const string& id) const {
    for (const auto& row : rows) {
        if (row.id == id)
            return true;
    }
    return false;
}

// getRowNumber(id)
int SymbolTable::getRowNumber(const string& id) const {
    for (const auto& row : rows) {
        if (row.id == id)
            return row.rowNumber;
    }
    return -1; // not found
}

// getRows()
const vector<SymbolRow>& SymbolTable::getRows() const {
    return rows;
}


string SymbolTable::print() const {
    ostringstream oss;
    cout << "\n===== SYMBOL TABLE =====\n";
    for (const auto& row : this->getRows()) {
        oss << "Row " << row.rowNumber
            << " -> " << row.id << endl;
    }
    return oss.str();
}

