#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/CSVreader.h"

using namespace std;

int main() {
    stemming::english_stem<> stem;
    unordered_map<wstring, int> lexicon;
    wifstream questions("../dataset/removed_newline_questions.csv");
    wifstream answers("../dataset/removed_newline_answers.csv");
    wifstream tags("../dataset/test_tags.csv");
    wofstream lexicon_file("../data_structures/lexicon.txt");
    vector<wstring> row;
    //skipping headings
    readNext(row, questions);
    int i = 0;
    wchar_t c;
    vector<wchar_t> word;

    while(!questions.eof()){
        readNext(row,questions);
        wstringstream title(row[5]);
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

        word.clear();
        wstringstream body(row[6]);
        while(body >> c){
            while((isalpha(c) || c == '\'') && !body.eof()) {
                if (c == '\'')
                    body >> noskipws >> c;
                word.push_back(tolower(c));
                body >> noskipws >> c;
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

    /*while(!answers.eof()) {
        readNext(row, answers);
        wstringstream title(row[5]);
        while (title >> c) {
            while ((isalpha(c) || c == '\'') && !title.eof()) {
                if (c == '\'')
                    title >> noskipws >> c;
                word.push_back(tolower(c));
                title >> noskipws >> c;
            }
            if (!word.empty()) {
                wstring Word(word.begin(), word.end());
                stem(Word);
                if (lexicon.count(Word) == 0)
                    lexicon[Word] = i++;
            }
            word.clear();
        }
    }
*/
    //writing lexicon to file
    for(auto& x : lexicon)
        lexicon_file << x.first << L"," << x.second << "\n";

    questions.close();
    lexicon_file.close();
    answers.close();
    tags.close();
}
