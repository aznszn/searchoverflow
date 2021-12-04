#include <sstream>
#include "string"
#include "vector"
#include "fstream"

using namespace std;
void removeCode(wstring& cell);

void readNext(vector<wstring>& row, wifstream& file){
    wstring line;
    getline(file, line);
    removeCode(line);
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

void removeCode(wstring& cell){
    //matching everything inside the code tag and a tag, also matching any html tags
    const wregex code(L"<code.*>.*?<\\/code>|<a.*>.*?<\\/a>|<.{1,10}>");
    //const wregex code(L"<code.*>.*?<\\/code>");
    //const wregex code(L"<.{1,10}>");
    //const wregex code(L"(<code>).*(</code>)");
    //const wregex code(L"\\\\<.*?\\\\>");
    cell = regex_replace(cell, code, L"");
}