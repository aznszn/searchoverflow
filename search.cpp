#include "stemminglib/english_stem.h"
#include <filesystem>
#include <chrono>
#include <sstream>

#define LINE_SIZE 201
#define WORDS_IN_BARRELS 500

using namespace std;
using namespace chrono;
using namespace filesystem;


vector<wstring> readNext(wifstream& file);
vector<vector<wstring>> fetch_table(wifstream& file);
vector<wstring> getAndParseQuery();
unordered_map<wstring, int> getLexicon();
unordered_map<wstring, vector<wstring>> getDocsInfo();
void getLineNums(vector<unordered_map<wstring, int>> &lineNums);

void customIntersection(vector<vector<pair<wstring,wstring>>> &);
void makeCombiUtil(vector<vector<int>>&, vector<int>&, int, int, int);

vector<vector<int> > makeCombi(int, int);


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

        if (!queryWordIDs.size()){
            cout << "Words do not exist in the lexicon" << endl;
            continue;
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


        //cout << "Number of words " << docs.size() << endl;

        for (auto &item: docs) {
            sort(item.begin(), item.end(), [&](const pair<wstring, wstring> &a, const pair<wstring, wstring> &b) {
                return stoi(a.first) < stoi(b.first);
            });
        }

        vector<vector<pair<wstring, wstring>>> beforeIntersectDocs = docs;
        customIntersection(docs);

        if(docs[0].size() == 0 && queryWordIDs.size() > 3){
            cout << "No intersection found" << endl;
            vector<vector<int>> combinations = makeCombi(queryWordIDs.size(), queryWordIDs.size()-2);

            int maxInDocsComb = INT_MIN;
            for (int i = 0; i < combinations.size(); ++i){
                vector<vector<pair<wstring, wstring>>> docsTemp;
                for (int j = 0; j < combinations[i].size(); ++j){
                    docsTemp.push_back(beforeIntersectDocs[j]);
                }
                customIntersection(docsTemp);
                int sum = 0;
                for (int k = 0; k < docsTemp.size(); ++k){
                    sum += docsTemp[k].size();
                }
                if (docsTemp[0].size() != 0 && sum > maxInDocsComb){
                    docs = docsTemp;
                    maxInDocsComb = sum;
                }
            }
        }

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
                wordIdImpPair.second = stoi(imp);
                wordIdImpPair.first = wordID;
                getline(linestream, imp, L',');
                wstring cell;
                while (getline(linestream, cell, L',')) {
                    int pos = stoi(cell);
                    hitPos.push_back(pos);
                    posWordMap[pos] = wordIdImpPair;
                }

            }
            std::sort(hitPos.begin(), hitPos.end());
            for (int index = 1; index < hitPos.size() - 1; ++index) {
                if (((hitPos[index] - hitPos[index - 1]) > 13) && ((hitPos[index + 1] - hitPos[index]) > 13)) {
                    hitPos.erase(hitPos.begin() + index);
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
                        score = ((double) bunchIDs.size() - 1) / score;
                        score *= ((double) bunchIDs.size())/(double)queryWordIDs.size();
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

        std::sort(docScores.begin(), docScores.end(), [&](const pair<wstring, double> &a, const pair<wstring, double> &b) {
            return (a.second > b.second);
        });

        cout << "\n" << duration_cast<milliseconds>(high_resolution_clock::now() - start).count()/1000.0 << " seconds to fetch " << docScores.size() << " results\n";
        for (auto &item: docScores) {
            wcout << "https://stackoverflow.com/questions/" << item.first << " " << "score " << item.second << "\n";
        }
    }
}
#pragma clang diagnostic pop

void getLineNums(vector<unordered_map<wstring, int>>& lineNums){
    auto dirIter = std::filesystem::directory_iterator(current_path().string() + "/../data_structures/f_index");
    int fileCount = count_if(
            begin(dirIter),
            end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
    );

    vector<wifstream> files;
    files.reserve(fileCount);
    for(int i = 0; i < fileCount; i++){
        files.emplace_back("../data_structures/f_index/" + to_string(i) + ".txt", ios::in);
    }
    int i = 0;

    vector<wstring> row;
    wstring line;
    wstring cell;
    for(auto& file : files){
        int ln = 1;
        lineNums.emplace_back(unordered_map<wstring, int>());
        getline(file, line); //new
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

vector<vector<int>> makeCombi(int n, int k){
    vector<vector<int> > ans;
    vector<int> tmp;
    makeCombiUtil(ans, tmp, n, 1, k);
    return ans;
}

void makeCombiUtil(vector<vector<int> >& ans, vector<int>& tmp, int n, int left, int k){
    // Pushing this vector to a vector of vector
    if (k == 0) {
        ans.push_back(tmp);
        return;
    }
    // i iterates from left to n. First time
    // left will be 1
    for (int i = left; i <= n; ++i)
    {
        tmp.push_back(i);
        makeCombiUtil(ans, tmp, n, i + 1, k - 1);

        // Popping out last inserted element
        // from the vector
        tmp.pop_back();
    }
}