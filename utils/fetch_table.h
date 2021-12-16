//
// Created by HP on 08/12/2021.
//

#ifndef UNTITLED_FETCH_TABLE_H
#define UNTITLED_FETCH_TABLE_H
#include <sstream>

using namespace std;
vector<wstring> readNext(wifstream& file);

vector<vector<wstring>> fetch_table(wifstream& file){
    vector<vector<wstring>> table;
    readNext(file);
    while(!file.eof())
        table.push_back(readNext(file));
    return table;
}

vector<wstring> readNext(wifstream& file){
    vector<wstring> row;
    wstring cell;
    wstring line;
    getline(file, line);
    wstringstream lineStream(line);
    while(getline(lineStream, cell, L','))
        row.push_back(cell);
    return row;
}

#endif //UNTITLED_FETCH_TABLE_H
