#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"

void buildLexicon(unordered_map<wstring, int> &lexicon, const vector<vector<wstring>> &table, int& i, int col);
using namespace std;

int main() {
    unordered_map<wstring, int> lexicon;
    wifstream questions("../dataset/questions_100k.csv");
    wifstream answers("../dataset/answers_100k.csv");
    wifstream tags("../dataset/test_tags.csv");
    wofstream lexicon_file("../data_structures/lexicon.txt");

    int i = 0;
    vector<vector<wstring>> table;

    fetch_table(table, questions);
    vector<int> indices = {5, 6};
    cout << "questions table fetched";
    parseHTML(table, indices);
    cout << "\nparsed html";
    buildLexicon(lexicon, table, i, 5);
    cout << "\nbuilt titles lexicon";
    buildLexicon(lexicon, table, i, 6);
    cout << "\nbuilt body lexicon";

    fetch_table(table, answers);
    cout << "\nanswers table fetched";
    indices.pop_back();
    parseHTML(table, indices);
    cout << "\nhtml parsed";
    buildLexicon(lexicon, table, i, 5);
    cout << "\nbuilt answers lexicon";

    /*fetch_table(table, tags);
    buildLexicon(lexicon, table, i, 1);*/


    //writing lexicon to file
    for (auto &x: lexicon)
        lexicon_file << x.first << L"," << x.second << "\n";

    questions.close();
    lexicon_file.close();
    answers.close();
    tags.close();
}

void buildLexicon(unordered_map<wstring, int> &lexicon, const vector<vector<wstring>> &table, int& i, int col) {
    wstring word;
    stemming::english_stem<> stem;
    int j = 0;
    while(j < table.size()){
        wstring title = table[j++][col];
        for(int k = 0; k < title.size(); ++k){
            while(isalpha(title[k]) && k < title.size()){
                word.push_back(tolower(title[k++]));
                k += title[k] == '\'';
            }
            if(!word.empty()){
                stem(word);
                if(lexicon.count(word) == 0)
                    lexicon[word] = i++;
            }
            word.clear();
        }

    }
}



