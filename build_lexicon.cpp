#include "stemminglib/english_stem.h"
#include "utils/fetch_table.h"
#include "utils/parseHTML.h"
#include "inverted_index.h"

#define WORDS_IN_FILE 500
#define MAX_WORD_LEN 17
#define TAGS_IMP 4
#define TITLE_IMP 10
#define BODY_IMP 1
#define LINECAP 200
#define QBODY_COL 6
#define QTITLE_COL 5
#define ANSBODY_COL 5
#define ANS_PID_COL 3
#define SCORE_COL 4

using namespace std;
using namespace filesystem;

void buildLexicon(unordered_map<wstring, int> &, vector<vector<wstring>>, vector<vector<wstring>>, vector<vector<wstring>>, int&, unordered_map<wstring, int> &);
void buildForwardIndex(unordered_map<wstring, int> &, unordered_map<wstring, int> &, int, const wstring&, unordered_map<wstring, wstring> &);
void utilLexiconFunction(unordered_map<wstring, int> &, unordered_map<wstring, int> &, wstringstream&, int &, unordered_map<wstring, wstring> &, int &, unordered_map<wstring, int> &);
int getAnswer(int &, vector<vector<wstring>> &, const wstring&);
int getTag(int &, vector<vector<wstring>>&, const wstring&);
void utilLexiconForTags(unordered_map<wstring, int> &, wstringstream &, int &, vector<wstring> &, unordered_map<wstring,int>&);
void buildForwardIndexTags(unordered_map<wstring, int> &, int, wstring &, vector<wstring> &);
void readStopWords(unordered_map<wstring, int> &);
void getLineNums();


//TODO: remove global
auto array_fi = new vector<wofstream>;
int main() {
    {
        path x(current_path().string() + "/../dataset/questions_100k.csv");
        path y(current_path().string() + "/../dataset/answers_100k.csv");
        path z(current_path().string() + "/../dataset/tags_100k.csv");

        wofstream metaData("../data_structures/metadata.txt");
        metaData << file_size(x) << endl << file_size(y) << endl << file_size(z);
        metaData.close();

        unordered_map<wstring, int> stopWordsLexicon;
        readStopWords(stopWordsLexicon);
        cout << "Stop Words Lexicon made" << endl;

        unordered_map<wstring, int> lexicon;
        wofstream lexicon_file("../data_structures/lexicon.txt", ios::out);

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
        //parseHTML(questions_table, QBODY_COL);
        //cout << "questions HTML parsed" << endl;
        //parseHTML(answers_table, ANSBODY_COL);
        //cout << "answers HTML parsed" << endl;

        buildLexicon(lexicon, questions_table, answers_table, tags_table, i, stopWordsLexicon);

        for (auto &x: *array_fi)
            x.close();

        cout << (*array_fi).size() << endl;

        delete array_fi;

        //writing lexicon to file
        for (auto &x: lexicon)
            lexicon_file << x.first << L"," << x.second << "\n";

        cout << "Lexicon file created" << endl;

        lexicon_file.close();
    }

    buildInverted();
    cout << "Built inverted" << endl;

    getLineNums();
    cout << "Got Line Num" << endl;
}

void buildLexicon(unordered_map<wstring, int> &lexicon, vector<vector<wstring>> questions, vector<vector<wstring>> answers, vector<vector<wstring>> tags, int& i, unordered_map<wstring, int> &stopWordsLexicon) {
    int j = 0; // for questions counter
    int k = 0; // for answers counter
    int t = 0; // for tags counter
    wofstream docList("../data_structures/docList.txt", ios::out);

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
            utilLexiconForTags(lexicon, row_stream, i, wordsInTags, stopWordsLexicon);
        }
        buildForwardIndexTags(lexicon, TAGS_IMP, questions[j][0], wordsInTags);

        // question titles
        auto row_stream = wstringstream(questions[j][QTITLE_COL]);
        utilLexiconFunction(lexicon, storesCount, row_stream, i, hits, overallCharacterCount, stopWordsLexicon);
        buildForwardIndex(lexicon, storesCount, TITLE_IMP, questions[j][0], hits);

        // questions body
        row_stream = wstringstream(questions[j][QBODY_COL]);
        utilLexiconFunction(lexicon, storesQACount, row_stream, i, hitsQA, overallCharacterCount, stopWordsLexicon);

        // answers for that specific question
        int max = INT_MIN;
        int answerCount = 0;
        while (1) {
            int ansFound = getAnswer(k, answers, questions[j][0]);

            if (ansFound == -1) {
                break;
            }
            ++answerCount;
            if (stoi(answers[ansFound][SCORE_COL]) > max){
                max = stoi(answers[ansFound][SCORE_COL]);
            }

            row_stream = wstringstream(answers[ansFound][ANSBODY_COL]);
            utilLexiconFunction(lexicon, storesQACount, row_stream, i, hitsQA, overallCharacterCount, stopWordsLexicon);
        }
        buildForwardIndex(lexicon, storesQACount, BODY_IMP, questions[j][0], hitsQA);

        double rank = 0.46 * max + 0.33 * stoi(questions[j][SCORE_COL]) + 0.21 * answerCount;
        docList << endl << questions[j][0] << L"," << overallCharacterCount << L"," << rank; // change to answerCount
        ++j;
    }
}

void utilLexiconFunction(unordered_map<wstring, int> &lexicon, unordered_map<wstring, int> &storesCount, wstringstream& row_stream, int &i, unordered_map<wstring, wstring> &hits, int &overallCharacterCount, unordered_map<wstring, int> &stopWordsLexicon){
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
                    if (word.length() <= MAX_WORD_LEN && !lexicon.count(word) && !stopWordsLexicon.count(word)) {
                        if (i % (WORDS_IN_FILE) == 0) {
                            array_fi->push_back(wofstream(
                                    "../data_structures/barrels/" + to_string(i / (WORDS_IN_FILE )) + ".txt",
                                    ios::out));
                        }
                        lexicon[word] = i++;
                        storesCount[word] = 1;

                        ++overallCharacterCount;
                        if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
                    }
                    else if (word.length() <= MAX_WORD_LEN && storesCount.count(word) && !stopWordsLexicon.count(word)){
                        storesCount[word] += 1;

                        ++overallCharacterCount;
                        if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
                    }
                    else if (word.length() <= MAX_WORD_LEN && !stopWordsLexicon.count(word)){
                        storesCount[word] = 1;

                        ++overallCharacterCount;
                        if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                            hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
                    }
                    word.clear();
                }

        }
    }
    if (!word.empty()) {
        stem(word);
        if (word.length() <= MAX_WORD_LEN && !lexicon.count(word) && !stopWordsLexicon.count(word)) {
            if (i % (WORDS_IN_FILE) == 0) {
                array_fi->push_back(wofstream(
                        "../data_structures/barrels/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
                        ios::out));
            }
            lexicon[word] = i++;
            storesCount[word] = 1;

            ++overallCharacterCount;
            if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
        }
        else if (word.length() <= MAX_WORD_LEN && storesCount.count(word) && !stopWordsLexicon.count(word)){
            storesCount[word] += 1;

            ++overallCharacterCount;
            if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
        }
        else if (word.length() <= MAX_WORD_LEN && !stopWordsLexicon.count(word)){
            storesCount[word] += 1;

            ++overallCharacterCount;
            if(hits[word].length() + to_wstring(overallCharacterCount).length() + 20 < LINECAP)
                hits[word] = hits[word] + L"," + to_wstring(overallCharacterCount);
        }
        word.clear();
    }
}

void utilLexiconForTags(unordered_map<wstring, int> &lexicon, wstringstream &row_stream, int &i, vector<wstring> &wordsInTags, unordered_map<wstring, int>& stopWordsLexicon){
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
                    if (word.length() <= MAX_WORD_LEN && !lexicon.count(word) && !stopWordsLexicon.count(word)) {
                        if (i % (WORDS_IN_FILE) == 0) {
                            array_fi->push_back(wofstream(
                                    "../data_structures/barrels/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
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
        if (word.length() <= MAX_WORD_LEN && !lexicon.count(word) && !stopWordsLexicon.count(word)) {
            if (i % (WORDS_IN_FILE) == 0) {
                array_fi->push_back(wofstream(
                        "../data_structures/barrels/" + to_string(i / (WORDS_IN_FILE)) + ".txt",
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
        wstringstream ss;
        ss  << endl << id << L"," << (wordId - (wordId/WORDS_IN_FILE)*WORDS_IN_FILE) << L"," << importance << L"," << x.second << hits[x.first];
        array_fi->at(wordId / (WORDS_IN_FILE)) << ss.str() << setw(LINECAP - ss.str().length()) << " ";
    }
}

void buildForwardIndexTags(unordered_map<wstring, int> &lexicon, int importance, wstring &id, vector<wstring> &wordsInTags){
    //documentId, wordId, importance
    for (auto &x : wordsInTags){
        int wordId = lexicon[x];
        wstringstream ss;
        ss  << "\n" << id << L"," << (wordId - (wordId/WORDS_IN_FILE)*WORDS_IN_FILE) << L"," << importance << 1 << L"," << 0;
        array_fi->at(wordId / (WORDS_IN_FILE)) << ss.str() << setw(LINECAP - ss.str().length()) << " ";
    }
}

inline int getAnswer(int &k, vector<vector<wstring>>& answers, const wstring& id){
    if(k < answers.size() && answers[k][ANS_PID_COL] == id)
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

// documentID, numberOfWords, question score, answers score max, number of answers
// answers score max 46%
// questions score 33%
// number of answers 21%

void readStopWords(unordered_map<wstring, int> &map){
    wstring word;
    stemming::english_stem<> stem;
    wifstream stopWordsFile("../dataset/stop_words_english.txt");
    while (!stopWordsFile.eof()){
        getline(stopWordsFile, word);
        stem(word);
        map[word] = 0;
    }
}

void getLineNums(){
    wofstream lineNumberFile("../data_structures/lineNumbers.txt", ios::out);

    auto dirIter = std::filesystem::directory_iterator(current_path().string() + "/../data_structures/barrels");
    int fileCount = count_if(
            begin(dirIter),
            end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
    );

    vector<wifstream> files;
    files.reserve(fileCount);
    for(int i = 0; i < fileCount; i++){
        files.emplace_back("../data_structures/barrels/" + to_string(i) + ".txt", ios::in);
    }
    int i = 0;

    vector<wstring> row;
    wstring line;
    for(auto& file : files){
        wstring cell;
        wstring temp;
        int ln = 1;
        getline(file, line); //new
        while(getline(file, line)){
            ln++;
            row.clear();
            wstringstream linestream(line);
            getline(linestream, cell, L',');
            getline(linestream, cell, L',');
            if(temp != cell) {
                lineNumberFile << cell << L',' << ln << '\n';
            }
            temp = cell;
        }
        file.close();
        i++;
    }
    lineNumberFile.close();
}