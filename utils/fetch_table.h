#include <vector>
#include <string>
#include <fstream>

using namespace std;
vector<wstring>& readNext(wstring& line, wstring& cell, vector<wstring>& row, wifstream& file);

void fetch_table(vector<vector<wstring>>& table, wifstream& file){
    table.clear();
    vector<wstring> row;
    wstring cell;
    wstring line;
    readNext(line,cell,row,file);
    while(!file.eof())
        table.push_back(readNext(line,cell,row,file));
}

inline vector<wstring>& readNext(wstring& line, wstring& cell, vector<wstring>& row, wifstream& file){
    getline(file, line);
    wstringstream lineStream(line);
    row.clear();
    while(getline(lineStream, cell, L','))
        row.push_back(cell);
    return row;
}