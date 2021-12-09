//
// Created by HP on 08/12/2021.
//
#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"
using namespace std;

void buildLexicon(unordered_map<wstring, int> &, vector<vector<wstring>>, vector<vector<wstring>>, int&);
void buildForwardIndex(unordered_map<wstring, int> &, unordered_map<wstring, int> &, int, wstring);
void utilLexiconFunction(unordered_map<wstring, int> &, unordered_map<wstring, int> &, wstringstream *, int &);
int getAnswer(int &, vector<vector<wstring>> &, wstring);

int overallCount = 0;
//wofstream *array_fi = new wofstream[500];
auto array_fi = new vector<wofstream>;
int main() {
    unordered_map<wstring, int> lexicon;
    wofstream lexicon_file("../data_structures/lexicon.txt", ios::out);
    cout << "lexicon file" << endl;


    //for (int i = 0; i < 500; ++i){
        //array_fi->at(i) = wofstream("../data_structures/forwardIndex" + to_string(i) + ".txt", ios::out);
    //}


    int i = 0;

        wifstream questions("../dataset/questions_100k.csv");
        vector<vector<wstring>> questions_table = fetch_table(questions);
        cout << "questions table fetched" << endl;

        wifstream answers("../dataset/answers_100k.csv");
        vector<vector<wstring>> answers_table = fetch_table(answers);
        cout << "answers table fetched" << endl;

        parseHTML(questions_table, 6);
        cout << "questions HTML parsed" << endl;
        parseHTML(answers_table, 5);
        cout << "answers HTML parsed" << endl;


        buildLexicon(lexicon, questions_table, answers_table, i);

        for(auto& x : *array_fi)
            x.close();

    //writing lexicon to file
    for (auto &x: lexicon)
        lexicon_file << x.first << L"," << x.second << "\n";

    lexicon_file.close();
}

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> questions, vector<vector<wstring>> answers, int& i) {
    const int TITLE = 5;
    const int BODY = 6;
    const int ANS_BODY = 5;

    int TITLE_IMP = 2;
    int BODY_IMP = 1;


    int j = 0;
    int k = 0;

    //wofstream forwardIndex("../data_structures/forwardIndex.txt");
    while(j < questions.size()) {
        unordered_map<wstring, int> storesCount;
        unordered_map<wstring, int> storesQACount;

        wstringstream *row_stream = new wstringstream(questions[j][5]);
        utilLexiconFunction(lexicon, storesCount, row_stream, i);
        buildForwardIndex(lexicon, storesCount, TITLE_IMP, questions[j][0]);

        row_stream = new wstringstream(questions[j][6]);
        utilLexiconFunction(lexicon, storesQACount, row_stream, i);

        while(1){
            int ansFound = getAnswer(k, answers, questions[j][0]);

            if (ansFound == -1){
                break;
            }
            //cout << "Answers" << k << endl;
            wstringstream *row_stream = new wstringstream(answers[ansFound][5]);
            utilLexiconFunction(lexicon, storesQACount, row_stream, i);
        }
        buildForwardIndex(lexicon, storesQACount, BODY_IMP, questions[j][0]);
        ++j;
    }
}

void utilLexiconFunction(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, wstringstream *row_stream, int &i){
    wstring word;
    stemming::english_stem<> stem;
    wchar_t c;

    while (*row_stream >> noskipws >> c) {
        switch (c) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
                word.push_back(tolower(c));
                break;
            case '\'':
                break;
            default:
                if (!word.empty()) {
                    stem(word);
                    if (word.length() <= 17 && !lexicon.count(word)) {
                        if(i % 501 == 0) {
                            array_fi->push_back(wofstream("../data_structures/f_index/" + to_string(i / 501) + ".txt", ios::out));
                        }
                        lexicon[word] = i++;
                        ++overallCount;
                        storesCount[word] = 1;
                    }
                    else if (word.length() <= 17 && storesCount.count(word)) {
                        storesCount[word] += 1;
                    }
                    else if (word.length() <= 17){
                        storesCount[word] = 1;
                    }
                    word.clear();
                }
        }
    }
    if (!word.empty()) {
        stem(word);
        if (word.length() <= 17 && !lexicon.count(word)){
            if(i % 501 == 0) {
                array_fi->push_back(wofstream("../data_structures/f_index/" + to_string(i / 501) + ".txt", ios::out));
            }
            lexicon[word] = i++;
            ++overallCount;
            storesCount[word] = 1;
        }
        else if (word.length() <= 17 && storesCount.count(word)) {
            storesCount[word] += 1;
        }
        else if (word.length() <= 17){
            storesCount[word] = 1;
        }
        word.clear();
    }
}

void buildForwardIndex(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, int importance, wstring id){
    for(auto &x: storesCount){
        int wordId = lexicon[x.first];
        //wofstream forwardIndex("../data_structures/forwardIndex" + to_string(wordId / 1000) + ".txt", ios::app);

        array_fi->at(wordId / 501) << id << L"," << lexicon[x.first] << L"," << x.second << L"," << importance << endl;
        //forwardIndex.close();
    }
}

int getAnswer(int &k, vector<vector<wstring>>& answers, wstring id){
    while (k < answers.size()){
        if (answers[k][3] == id){
            return k++;
        }
        else{
            return -1;
        }
    }
    return -1;
}


