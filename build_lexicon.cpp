#include "bits/stdc++.h"
#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"
#include <dirent.h>
#include <sys/types.h>

#define WORDS_IN_FILE 500
#define MAX_WORD_LEN 17
#define TAGS_IMP 3
#define TITLE_IMP 2
#define BODY_IMP 1

using namespace std;


void buildLexicon(unordered_map<wstring, int> &, vector<vector<wstring>>, vector<vector<wstring>>, vector<vector<wstring>>, int&);
void buildForwardIndex(unordered_map<wstring, int> &, unordered_map<wstring, int> &, int, const wstring&, unordered_map<wstring, wstring> &);
void utilLexiconFunction(unordered_map<wstring, int> &, unordered_map<wstring, int> &, wstringstream&, int &, unordered_map<wstring, wstring> &, int &);
int getAnswer(int &, vector<vector<wstring>> &, const wstring&);
int getTag(int &, vector<vector<wstring>>&, const wstring&);
void utilLexiconForTags(unordered_map<wstring, int> &, wstringstream &, int &, vector<wstring> &);
void buildForwardIndexTags(unordered_map<wstring, int> &, int, wstring &, vector<wstring> &);
void buildInvertedIndex();

//TODO: remove global
auto array_fi = new vector<wofstream>;
int main() {
    {
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
        cout << answers_table.size();

        wifstream tags("../dataset/tags_100k.csv");
        vector<vector<wstring>> tags_table = fetch_table(tags);
        cout << "tags table fetched" << endl;
        cout << tags_table.size();
        parseHTML(questions_table, 6);
        cout << "questions HTML parsed" << endl;
        parseHTML(answers_table, 5);
        cout << "answers HTML parsed" << endl;

        buildLexicon(lexicon, questions_table, answers_table, tags_table, i);

        for (auto &x: *array_fi)
            x.close();

        cout << (*array_fi).size() << endl;

        delete array_fi;

        //writing lexicon to file
        for (auto &x: lexicon)
            lexicon_file << x.first << L"," << x.second << "\n";

        lexicon_file.close();
    }
    buildInvertedIndex();
    cout<< "end";
}

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> questions, vector<vector<wstring>> answers, vector<vector<wstring>> tags, int& i) {
    int j = 0; // for questions counter
    int k = 0; // for answers counter
    int t = 0; // for tags counter

    while (j < questions.size()) {
        int overallCharacterCount = 0;
        unordered_map<wstring, int> storesCount;
        unordered_map<wstring, int> storesQACount;
        unordered_map<wstring, wstring> hits;
        unordered_map<wstring, wstring> hitsQA;
        vector<wstring> wordsInTags;

        // tags
        while(1) {
            int tagFound = getTag(t, tags, questions[j][0]);

            if (tagFound == -1){
                break;
            }
            auto row_stream = wstringstream(tags[tagFound][1]);
            utilLexiconForTags(lexicon, row_stream, i, wordsInTags);
        }
        buildForwardIndexTags(lexicon, TAGS_IMP, questions[j][0], wordsInTags);

        // questions titles
        auto row_stream = wstringstream(questions[j][5]);
        utilLexiconFunction(lexicon, storesCount, row_stream, i, hits, overallCharacterCount);
        buildForwardIndex(lexicon, storesCount, TITLE_IMP, questions[j][0], hits);

        // questions body
        row_stream = wstringstream(questions[j][6]);
        utilLexiconFunction(lexicon, storesQACount, row_stream, i, hitsQA, overallCharacterCount);

        // answers for that specific question
        while (1) {
            int ansFound = getAnswer(k, answers, questions[j][0]);

            if (ansFound == -1) {
                break;
            }
            row_stream = wstringstream(answers[ansFound][5]);
            utilLexiconFunction(lexicon, storesQACount, row_stream, i, hitsQA, overallCharacterCount);
        }
        buildForwardIndex(lexicon, storesQACount, BODY_IMP, questions[j][0], hitsQA);

        // increment to next question
        ++j;
    }
}

void utilLexiconFunction(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, wstringstream& row_stream, int &i, unordered_map<wstring, wstring> &hits, int &overallCharacterCount){
    wstring word;
    stemming::english_stem<> stem;
    wchar_t c;

    while (row_stream >> noskipws >> c) {
        ++overallCharacterCount;
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
                        if (i % (WORDS_IN_FILE) == 0) {
                            array_fi->push_back(wofstream(
                                    "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
                                    ios::out));
                        }
                        lexicon[word] = i++;
                        storesCount[word] = 1;

                        hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
                    }
                    else if (word.length() <= MAX_WORD_LEN && storesCount.count(word)){
                        storesCount[word] += 1;

                        hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
                    }
                    else if (word.length() <= MAX_WORD_LEN){
                        storesCount[word] = 1;

                        hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
                    }
                    word.clear();
                }

        }
    }
    if (!word.empty()) {
        stem(word);
        if (word.length() <= MAX_WORD_LEN && !lexicon.count(word)) {
            if (i % (WORDS_IN_FILE) == 0) {
                array_fi->push_back(wofstream(
                        "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
                        ios::out));
            }
            lexicon[word] = i++;
            storesCount[word] = 1;

            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
        }
        else if (word.length() <= MAX_WORD_LEN && storesCount.count(word)){
            storesCount[word] += 1;

            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
        }
        else if (word.length() <= MAX_WORD_LEN){
            storesCount[word] += 1;

            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount - word.length());
        }
        word.clear();
    }
}

void utilLexiconForTags(unordered_map<wstring, int> &lexicon, wstringstream &row_stream, int &i, vector<wstring> &wordsInTags){
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
                        if (i % (WORDS_IN_FILE) == 0) {
                            array_fi->push_back(wofstream(
                                    "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
                                    ios::out));
                        }
                        lexicon[word] = i++;
                        wordsInTags.push_back(word);
                    }
                    word.clear();
                }

        }
    }
    if (!word.empty()) {
        stem(word);
        if (word.length() <= MAX_WORD_LEN && !lexicon.count(word)) {
            if (i % (WORDS_IN_FILE) == 0) {
                array_fi->push_back(wofstream(
                        "../data_structures/f_index/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
                        ios::out));
            }
            lexicon[word] = i++;
            wordsInTags.push_back(word);
        }
        word.clear();
    }
}

void buildForwardIndex(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, int importance, const wstring& id, unordered_map<wstring, wstring> &hits){
    //documentId, wordId, importance, numberOfHits, hits
    for(auto &x: storesCount){
        int wordId = lexicon[x.first];
        array_fi->at(wordId / (WORDS_IN_FILE)) << endl << id << L"," << (wordId - (wordId/WORDS_IN_FILE)*500) << L"," << importance << L"," << x.second << hits[x.first];
    }
}

void buildForwardIndexTags(unordered_map<wstring, int> &lexicon, int importance, wstring &id, vector<wstring> &wordsInTags){
    //documentId, wordId, importance
    for (auto &x : wordsInTags){
        int wordId = lexicon[x];
        array_fi->at(wordId / (WORDS_IN_FILE )) << endl << id << L"," << (wordId - (wordId/WORDS_IN_FILE)*500) << L"," << importance;
    }
}

inline int getAnswer(int &k, vector<vector<wstring>>& answers, const wstring& id){
    if(k < answers.size() && answers[k][3] == id)
        return k++;
    else
        return -1;
}

inline int getTag(int &t, vector<vector<wstring>>& tags, const wstring& id){
    if (t < tags.size() && tags[t][0] == id){
        return t++;
    }
    else {
        return -1;
    }
}

void buildInvertedIndex()
    {
        using namespace std::filesystem;
        string path = R"(C:\searchoverflow\data_structures\f_index)";

        for(const auto& entry : directory_iterator(path)) {
            wifstream curr(entry.path());
            vector<vector<wstring>> f_index_file = fetch_table(curr);

            auto sorted = vector<vector<wstring>>(f_index_file.size());
            vector<int> elem_array(1000, 0);

            for(auto& x : f_index_file)
                elem_array[stoi(x[1])]++;

            for(int i = 1; i < 1000; ++i)
                elem_array[i] += elem_array[i-1];

            for(auto & i : f_index_file)
                sorted.at(elem_array[stoi(i[1])]-- - 1) = i;

            wofstream currOut(entry.path(), ios::out);
            for(auto& row: sorted){
                for(auto& column : row)
                    currOut << column << ",";
                currOut.seekp(-1, ios::cur);
                currOut << "\n";
            }
        }
    }
