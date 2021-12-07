#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> table, int& i, int col);
using namespace std;

int main() {
    unordered_map<wstring, int> lexicon;
    wofstream lexicon_file("../data_structures/lexicon.txt");


    int i = 0;
    {
        wifstream questions("../dataset/questions_100k.csv");
        vector<vector<wstring>> questions_table = fetch_table(questions);
        parseHTML(questions_table, 6);
        buildLexicon(lexicon, questions_table, i, 5);
        cout << "\nbuilt titles lexicon";
        buildLexicon(lexicon, questions_table, i, 6);
        cout << "\nbuilt body lexicon";
        questions.close();
    }

    {
        wifstream answers("../dataset/answers_100k.csv");
        vector<vector<wstring>> answers_table = fetch_table(answers);
        cout << "\nanswers table fetched";
        parseHTML(answers_table, 5);
        cout << "\nhtml parsed";
        buildLexicon(lexicon, answers_table, i, 5);
        cout << "\nbuilt answers lexicon";
        answers.close();
    }

    {
        wifstream tags("../dataset/test_tags.csv");
        tags.close();
    }

    //writing lexicon to file
    for (auto &x: lexicon)
      lexicon_file << x.first << L"," << x.second << "\n";

    lexicon_file.close();
}

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> table, int& i, int col) {
    wstring word;
    stemming::english_stem<> stem;
    int j = 0;
    wchar_t c;
    while(j < table.size()){
        wstringstream row_stream (table[j++][col]);

        while(row_stream >> noskipws >> c){
            switch (c) {
                case 'a' ... 'z':
                case 'A' ... 'Z':
                    word.push_back(tolower(c));
                    break;
                case '\'':
                    break;
                default:
                    if(!word.empty()) {
                        stem(word);
                        if (word.length() <= 17 && !lexicon.count(word))
                            lexicon[word] = i++;
                        word.clear();
                    }
            }
        }
        if(!word.empty()) {
            stem(word);
            if (word.length() <= 17 && !lexicon.count(word))
                lexicon[word] = i++;
            word.clear();
        }
    }
}