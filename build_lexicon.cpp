#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"

#define WORDS_IN_FILE 500
#define MAX_WORD_LEN 17
#define TAGS_IMP 3
#define TITLE_IMP 2
#define BODY_IMP 1


using namespace std;

void buildLexicon(unordered_map<wstring, int> &, vector<vector<wstring>>, vector<vector<wstring>>, vector<vector<wstring>>, int&);
void buildForwardIndex(unordered_map<wstring, int> &, unordered_map<wstring, int> &, int, const wstring&);
void utilLexiconFunction(unordered_map<wstring, int> &, unordered_map<wstring, int> &, wstringstream&, int &);
int getAnswer(int &, vector<vector<wstring>> &, const wstring&);


//TODO: remove global
auto array_fi = new vector<wofstream>;
int main() {
    unordered_map<wstring, int> lexicon;
    wofstream lexicon_file("../data_structures/lexicon.txt", ios::out);
    cout << "lexicon file" << endl;

    int i = 0;

    wifstream questions("../dataset/questions_100k.csv");
    vector<vector<wstring>> questions_table = fetch_table(questions);
    cout << "questions table fetched" << endl;

    wifstream answers("../dataset/answers_100k.csv");
    vector<vector<wstring>> answers_table = fetch_table(answers);
    cout << "answers table fetched" << endl;

    wifstream tags("../dataset/tags_100k.csv");
    vector<vector<wstring>> tags_table = fetch_table(tags);
    cout << "tags table fetched" << endl;

    parseHTML(questions_table, 6);
    cout << "questions HTML parsed" << endl;
    parseHTML(answers_table, 5);
    cout << "answers HTML parsed" << endl;


    buildLexicon(lexicon, questions_table, answers_table, tags_table, i);

    for (auto &x: *array_fi)
        x.close();

    delete array_fi;
    //writing lexicon to file
    for (auto &x: lexicon)
        lexicon_file << x.first << L"," << x.second << "\n";

    lexicon_file.close();
}

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> questions, vector<vector<wstring>> answers, vector<vector<wstring>> tags, int& i) {
    int j = 0;
    int k = 0;

    while (j < questions.size()) {
        unordered_map<wstring, int> storesCount;
        unordered_map<wstring, int> storesQACount;

        auto row_stream = wstringstream(questions[j][5]);
        utilLexiconFunction(lexicon, storesCount, row_stream, i);
        buildForwardIndex(lexicon, storesCount, TITLE_IMP, questions[j][0]);
        row_stream = wstringstream(questions[j][6]);
        utilLexiconFunction(lexicon, storesQACount, row_stream, i);

        while (1) {
            int ansFound = getAnswer(k, answers, questions[j][0]);

            if (ansFound == -1) {
                break;
            }
            row_stream = wstringstream(answers[ansFound][5]);
            utilLexiconFunction(lexicon, storesQACount, row_stream, i);
        }
        buildForwardIndex(lexicon, storesQACount, BODY_IMP, questions[j][0]);
        ++j;
    }
}

void utilLexiconFunction(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, wstringstream& row_stream, int &i){
    wstring word;
    stemming::english_stem<> stem;
    wchar_t c;

    while (row_stream >> noskipws >> c) {
        switch (c) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '+':
            case '#':
                word.push_back(tolower(c));
                break;
            case '\'':
                break;
            default:
                if (!word.empty()) {
                    stem(word);
                    if (word.length() <= MAX_WORD_LEN && !lexicon.count(word)) {
                        if (i % (WORDS_IN_FILE + 1) == 0) {
                            array_fi->push_back(wofstream(
                                    "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE + 1)) + ".txt",
                                    ios::out));
                        }
                        lexicon[word] = i++;
                        storesCount[word] = 1;
                    }
                    else if (word.length() <= MAX_WORD_LEN && storesCount.count(word)){
                        storesCount[word] += 1;
                    }
                    else if (word.length() <= MAX_WORD_LEN){
                        storesCount[word] = 1;
                    }
                    word.clear();
                }

        }
    }
    if (!word.empty()) {
        stem(word);
        if (word.length() <= MAX_WORD_LEN && !lexicon.count(word)) {
            if (i % (WORDS_IN_FILE + 1) == 0) {
                array_fi->push_back(wofstream(
                        "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE + 1)) + ".txt",
                        ios::out));
            }
            lexicon[word] = i++;
            storesCount[word] = 1;
        }
        else if (word.length() <= MAX_WORD_LEN && storesCount.count(word)){
            storesCount[word] += 1;
        }
        else if (word.length() <= MAX_WORD_LEN){
            storesCount[word] += 1;
        }
        word.clear();
    }
}

void buildForwardIndex(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, int importance, const wstring& id){
    for(auto &x: storesCount){
        int wordId = lexicon[x.first];
        array_fi->at(wordId / (WORDS_IN_FILE + 1)) << id << L"," << lexicon[x.first] << L"," << x.second << L"," << importance << endl;
    }
}

inline int getAnswer(int &k, vector<vector<wstring>>& answers, const wstring& id){
    if(k < answers.size() && answers[k][3] == id)
        return k++;
    else
        return -1;
}