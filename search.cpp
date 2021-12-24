#include "stemminglib/english_stem.h"
#include <chrono>
#include <sstream>

#define LINE_SIZE 201
#define WORDS_IN_BARRELS 500

using namespace std;
using namespace std::chrono;

vector<wstring> readNext(wifstream& file);
vector<vector<wstring>> fetch_table(wifstream& file);
vector<wstring> getAndParseQuery();
unordered_map<wstring, int> getLexicon();
unordered_map<wstring, vector<wstring>> getDocsInfo();
void getLineNums(vector<unordered_map<wstring, int>> &lineNums);

void customIntersection(vector<vector<pair<wstring,wstring>>> &);
#pragma clang diagnostic push

#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    unordered_map<wstring, vector<wstring>> docs_info = getDocsInfo();
    vector<unordered_map<wstring, int>> lineNums;
    getLineNums(lineNums);
    unordered_map<wstring, int> lexicon = getLexicon();

    while (true) {

        vector<wstring> words = getAndParseQuery();
        vector<vector<pair<wstring, wstring>>> docs;
        vector<int> queryWordIDs;

        for (auto &w: words) {
            if (lexicon.count(w)) {
                queryWordIDs.push_back(lexicon[w]);
            }
        }

        auto start = high_resolution_clock::now();

        for (auto &ID: queryWordIDs) {
            vector<pair<wstring, wstring>> BIGG_APPLE;
            wstring idstr = to_wstring(ID % WORDS_IN_BARRELS);
            int lineNum = lineNums[(ID / WORDS_IN_BARRELS)][idstr];

            wifstream barrel("../data_structures/f_index/" + to_string(ID / WORDS_IN_BARRELS) + ".txt");
            barrel.seekg((lineNum - 1) * LINE_SIZE);

            wstring docID;
            wstring id_in_barrel;
            wstring info;

            while (getline(barrel, docID, L',')) {
                getline(barrel, id_in_barrel, L',');
                if (id_in_barrel.size() > idstr.size() ||
                    (id_in_barrel.size() == idstr.size() && id_in_barrel > idstr)) {
                    break;
                }
                getline(barrel, info, L' ');

                pair<wstring, wstring> pair1;
                pair1.first = docID;
                pair1.second = id_in_barrel;
                pair1.second.append(L"," + info);
                BIGG_APPLE.push_back(pair1);
                getline(barrel, docID);
            }
            barrel.close();
            docs.push_back(BIGG_APPLE);
        }
        //cout << "\n" << duration_cast<microseconds>(high_resolution_clock::now() - start).count() / 1000 << "ms to fetch results\n";

        cout << "Number of words " << docs.size() << endl;

        for (auto &item: docs) {
            sort(item.begin(), item.end(), [&](const pair<wstring, wstring> &a, const pair<wstring, wstring> &b) {
                return stoi(a.first) < stoi(b.first);
            });
        }

        customIntersection(docs);

        vector<pair<wstring, double>> docScores;

        int maxi = 0;
        for(int i = 0; i < docs.size(); i++){
            if(docs[i].size() > docs[maxi].size())
                maxi = i;
        }

        swap(docs[0], docs[maxi]);

        for (int i = 0; i < docs[0].size();) {
            wstring x = docs[0][i].first;
            vector<wstring> hitsVector;
            while (docs[0][i].first == x && i < docs[0].size()) {
                hitsVector.push_back(docs[0][i++].second);
            }
            for (int j = 1; j < docs.size(); ++j) {
                for (auto &z: docs[j])
                    if (z.first == x) {
                        hitsVector.push_back(z.second);
                    }
            }
            unordered_map<int, pair<wstring, int>> posWordMap;
            vector<int> hitPos;
            for (auto &hitlist: hitsVector) {
                wstringstream linestream(hitlist);
                pair<wstring, int> wordIdImpPair;
                wstring wordID;
                getline(linestream, wordID, L',');
                wstring imp;
                getline(linestream, imp, L',');
                getline(linestream, imp, L',');
                wordIdImpPair.first = wordID;
                wordIdImpPair.second = stoi(imp);
                wstring cell;
                while (getline(linestream, cell, L',')) {
                    int pos = stoi(cell);
                    hitPos.push_back(pos);
                    posWordMap[pos] = wordIdImpPair;
                }
                std::sort(hitPos.begin(), hitPos.end());
                for (int index = 1; index < hitPos.size() - 1; ++index) {
                    if (((hitPos[index] - hitPos[index - 1]) > 13) && ((hitPos[index + 1] - hitPos[index]) > 13)) {
                        hitPos.erase(hitPos.begin() + index);
                    }
                }
            }

            vector<wstring> bunchIDs;
            double cumscore = 0;
            int startImp = posWordMap[hitPos[0]].second;
            for (int k = 0; k < hitPos.size(); ++k) {
                if ((!bunchIDs.empty() && find_if(bunchIDs.begin(), bunchIDs.end(), [&](const wstring &a) {
                    if (posWordMap[hitPos[k]].second != startImp) {
                        startImp = posWordMap[hitPos[k]].second;
                        return true;
                    }
                    return a == posWordMap[hitPos[k]].first;
                }) != bunchIDs.end()) || k >= hitPos.size()) {
                    double score = 0;
                    for (int j = (k - bunchIDs.size() + 1); j < k; ++j) {
                        score += hitPos[j] - hitPos[j - 1];
                    }
                    if (bunchIDs.size() > 1) {
                        score = ((double) bunchIDs.size()) / score;
                        score /= (double) queryWordIDs.size();
                        score *= posWordMap[hitPos[k - 1]].second;
                        cumscore += score;
                    }
                    bunchIDs.clear();
                } else {
                    bunchIDs.push_back(posWordMap[hitPos[k]].first);
                }
            }
            docScores.emplace_back(docs[0][i - 1].first, cumscore);
        }

        std::sort(docScores.begin(), docScores.end(), [&](const pair<wstring, int> &a, const pair<wstring, int> &b) {
            return (a.second > b.second);
        });

        for (auto &item: docScores) {
            wcout << "https://stackoverflow.com/questions/" << item.first << "\n";
        }

    }
}
#pragma clang diagnostic pop

void getLineNums(vector<unordered_map<wstring, int>>& lineNums){
    vector<wifstream> files;
    files.reserve(311);
    for(int i = 0; i < 311; i++){
        files.emplace_back("../data_structures/f_index/" + to_string(i) + ".txt", ios::in);
    }
    int i = 0;

    vector<wstring> row;
    wstring line;
    wstring cell;
    for(auto& file : files){
        int ln = 0;
        lineNums.emplace_back(unordered_map<wstring, int>());
        while(getline(file, line)){
            ln++;
            row.clear();
            wstringstream linestream(line);
            getline(linestream, cell, L',');
            getline(linestream, cell, L',');
            if(!lineNums[i].count(cell)) {
                lineNums[i][cell] = ln;
            }
        }
        file.close();
        i++;
    }
}

unordered_map<wstring, vector<wstring>> getDocsInfo() {
    unordered_map<wstring, vector<wstring>> docs_info;
    wifstream doclist("../data_structures/doclist.txt");
    docs_info.reserve(100000);
    wstring word;
    wstring str;
    getline(doclist, word);
    word.clear();
    while(getline(doclist, word, L',')){
        vector<wstring> info;
        getline(doclist, str, L',');
        info.push_back(str);
        getline(doclist, str);
        info.push_back(str);
        docs_info[word] = info;
    }
    doclist.close();
    return docs_info;
}

unordered_map<wstring, int> getLexicon() {
    wifstream lexicon_in("../data_structures/lexicon.txt");
    unordered_map<wstring, int> lexicon;
    wstring word;
    int wordId;
    lexicon.reserve(156000);
    while(getline(lexicon_in, word, L',')){
        lexicon_in >> wordId;
        lexicon[word] = wordId;
        getline(lexicon_in, word);
    }
    lexicon_in.close();
    return lexicon;
}

vector<wstring> getAndParseQuery() {
    wstring word;
    wcout << "Enter query:\n";
    wstring query;
    getline(wcin, query);

    wstringstream query_stream(query);

    stemming::english_stem<> stem;
    wchar_t c;
    vector<wstring> words;
    word.clear();

    while(query_stream >> noskipws >> c){
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
                if(!word.empty()){
                    stem(word);
                    words.push_back(word);
                    word.clear();
                }
        }
    }

    if(!word.empty()){
        stem(word);
        words.push_back(word);
        word.clear();
    }
    return words;
}

void customIntersection(vector<vector<pair<wstring,wstring>>> &documents){
    if (documents.size() == 1){return;}
    vector<wstring> commonDocIDs;
    for (int i = 0; i < documents[0].size(); ++i){
        vector<wstring> temp;
        for (int z = 1; z < documents.size(); ++z){
            if(binary_search(documents[z].begin(), end(documents[z]), documents[0][i], [](const pair<wstring, wstring>& a, const pair<wstring, wstring>& b){
                return stoi(a.first) < stoi(b.first);
            })){
                temp.push_back(documents[0][i].first);
            }
        }
        if (temp.size() == documents.size() - 1){commonDocIDs.push_back(documents[0][i].first);}
    }

    for (int i = 0; i < documents.size(); ++i){
        documents[i].erase(remove_if(documents[i].begin(), documents[i].end(),
                                     [&commonDocIDs](const pair<wstring, wstring> &s) { return find(commonDocIDs.begin(), commonDocIDs.end(), s.first) == commonDocIDs.end(); }), documents[i].end());
    }
}