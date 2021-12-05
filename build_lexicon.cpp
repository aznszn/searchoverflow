#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"

void buildLexicon(unordered_map<wstring, int> &lexicon, const vector<vector<wstring>> &table, int i, int col);
using namespace std;

int main() {
    unordered_map<wstring, int> lexicon;
    wifstream questions("../dataset/removed_newline_questions.csv");
    wifstream answers("../dataset/removed_newline_answers.csv");
    wifstream tags("../dataset/test_tags.csv");
    wofstream lexicon_file("../data_structures/lexicon.txt");

    int i = 0;
    vector<vector<wstring>> table;

    fetch_table(table,questions);
    vector<int> indices = {5,6};
    parseHTML(table, indices);
    buildLexicon(lexicon, table, i, 5);
    buildLexicon(lexicon, table, i, 6);

    fetch_table(table,answers);
    indices.pop_back();
    parseHTML(table, indices);
    buildLexicon(lexicon, table, i, 5);

    fetch_table(table, tags);
    buildLexicon(lexicon, table, i, 1);


    //writing lexicon to file
    for(auto& x : lexicon)
        lexicon_file << x.first << L"," << x.second << "\n";

    questions.close();
    lexicon_file.close();
    answers.close();
    tags.close();
}

void buildLexicon(unordered_map<wstring, int> &lexicon, const vector<vector<wstring>> &table, int i, int col) {
    vector<wchar_t> word;
    stemming::english_stem<> stem;
    wchar_t c;
    int j = 0;
    while(j < table.size()){
        wstringstream title(table[j++][col]);
        while(title >> c){
            while((isalpha(c) || c == '\'') && !title.eof()){
                if(c=='\'')
                    title >> noskipws >> c;
                word.push_back(tolower(c));
                title >> noskipws >> c;
            }
            if(!word.empty()) {
                wstring Word (word.begin(),word.end());
                stem(Word);
                if(lexicon.count(Word) == 0)
                    lexicon[Word] = i++;
            }
            word.clear();
        }
    }
}



