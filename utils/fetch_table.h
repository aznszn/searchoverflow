#include <vector>
#include <string>
#include <fstream>

using namespace std;
void readNext(vector<wstring>& row, wifstream& file);

void fetch_table(vector<vector<wstring>>& table, wifstream& file){
    table.clear();
    vector<wstring> row;
    readNext(row,file);
    while(!file.eof()){
        readNext(row,file);
        table.push_back(row);
    }
}

void readNext(vector<wstring>& row, wifstream& file){
    wstring line;
    getline(file, line);
    wstringstream lineStream(line);
    vector<wchar_t> cell;
    row.clear();
    wchar_t c;
    while(lineStream  >> c) {
        cell.clear();
        while(c != ',' && !lineStream.eof()){
            if(c == '\"') {
                while (lineStream >> c && c != '\"')
                    cell.push_back(c);
            }
            else
                cell.push_back(c);
            lineStream >> noskipws >> c;
        }
        row.emplace_back(cell.begin(), cell.end());
    }
}