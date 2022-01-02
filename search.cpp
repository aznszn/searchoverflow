#include "stemminglib/english_stem.h"
#include <filesystem>
#include <chrono>
#include <sstream>
#include <unordered_set>

#define LINE_SIZE 201
#define WORDS_IN_BARRELS 500

using namespace std;
using namespace chrono;
using namespace filesystem;


vector<wstring> getAndParseQuery();
unordered_map<wstring, int> getLexicon();
unordered_map<wstring, vector<wstring>> getDocsInfo();
vector<unordered_map<wstring, int>> getLineNumbers();
void customIntersection(vector<vector<pair<wstring,wstring>>> &);
void makeCombiUtil(vector<vector<int>>&, vector<int>&, int, int, int);
vector<vector<int> > makeCombi(int, int);
double getCumScore(const vector<int> &queryWordIDs, unordered_map<int, pair<wstring, int>> &posWordMap, const vector<int> &hitPos);

#pragma clang diagnostic push

#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    unordered_map<wstring, vector<wstring>> docs_info = getDocsInfo();  //doclis
    vector<unordered_map<wstring, int>> lineNums = getLineNumbers(); // map that tells you where each word starts in the barrels
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

        if (queryWordIDs.empty()){
            cout << "Words do not exist in the lexicon" << endl;
            continue;
        }

        auto start = high_resolution_clock::now();
        for (auto &ID: queryWordIDs) {
            vector<pair<wstring, wstring>> BIGG_APPLE;
            wstring idstr = to_wstring(ID % WORDS_IN_BARRELS);
            int lineNum = lineNums[(ID / WORDS_IN_BARRELS)][idstr];

            wifstream barrel("../data_structures/barrels/" + to_string(ID / WORDS_IN_BARRELS) + ".txt");
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

                BIGG_APPLE.emplace_back(docID, id_in_barrel.append(L"," + info));
                getline(barrel, docID);
            }
            barrel.close();
            docs.push_back(BIGG_APPLE);
        }

        for (auto &item: docs) {
            sort(item.begin(), item.end(), [&](const pair<wstring, wstring> &a, const pair<wstring, wstring> &b) {
                return  a.first.size() < b.first.size() || (a.first.size() == b.first.size() && a.first < b.first);
            });
        }

        vector<pair<wstring, double>> docScores;

        if(queryWordIDs.size() > 1) {
            vector<vector<pair<wstring, wstring>>> beforeIntersectDocs = docs;
            customIntersection(docs);

            if (docs[0].empty() && queryWordIDs.size() > 3) {
                cout << "No intersection found" << endl;
                vector<vector<int>> combinations = makeCombi(queryWordIDs.size(), queryWordIDs.size() - 2);

                int maxInDocsComb = INT_MIN;
                for (auto &combination: combinations) {
                    vector<vector<pair<wstring, wstring>>> docsTemp;
                    docsTemp.reserve(combination.size());
                    for (int j = 0; j < combination.size(); ++j) {
                        docsTemp.push_back(beforeIntersectDocs[j]);
                    }
                    customIntersection(docsTemp);
                    int sum = 0;
                    for (auto &k: docsTemp) {
                        sum += k.size();
                    }
                    if (!docsTemp[0].empty() && sum > maxInDocsComb) {
                        docs = docsTemp;
                        maxInDocsComb = sum;
                    }
                }
            }


            int maxi = 0;
            for (int i = 0; i < docs.size(); i++) {
                if (docs[i].size() > docs[maxi].size()) {
                    maxi = i;
                }
            }
            swap(docs[0], docs[maxi]);

            for (int i = 0; i < docs[0].size();) {
                wstring docID = docs[0][i].first;
                vector<wstring> hitsVector;
                while (docs[0][i].first == docID && i < docs[0].size()) {
                    hitsVector.push_back(docs[0][i++].second);
                }

                for (int j = 1; j < docs.size(); ++j) {
                    for (auto &z: docs[j])
                        if (z.first == docID) {
                            hitsVector.push_back(z.second);
                        }
                }

                unordered_map<int, pair<wstring, int>> posWordMap;
                vector<int> hitPos;

                for (auto &hitlist: hitsVector) {
                    wstringstream linestream(hitlist);
                    wstring wordID;
                    wstring imp;
                    getline(linestream, wordID, L',');
                    getline(linestream, imp, L',');
                    wstring cell;
                    getline(linestream, cell, L',');

                    while (getline(linestream, cell, L',')) {
                        int pos = stoi(cell);
                        hitPos.push_back(pos);
                        posWordMap.emplace(pos, pair(wordID, stoi(imp)));
                    }

                }

                sort(hitPos.begin(), hitPos.end());

                for (int index = 1; index < hitPos.size() - 1; ++index) {
                    if (((hitPos[index] - hitPos[index - 1]) > 13) && ((hitPos[index + 1] - hitPos[index]) > 13)) {
                        hitPos.erase(hitPos.begin() + index);
                    }
                }

                double cumscore = 0.992*getCumScore(queryWordIDs, posWordMap, hitPos) + 0.008*stod(docs_info[docs[0][i-1].first][1]);
                docScores.emplace_back(docs[0][i - 1].first, cumscore);
            }
        }

        else{
            for(int i = 0; i < docs[0].size();){
                wstring docID = docs[0][i].first;
                vector<wstring> hitsVector;
                while (docs[0][i].first == docID && i < docs[0].size()) {
                    hitsVector.push_back(docs[0][i++].second);
                }

                double cumScore = 0;
                for (auto &hitlist: hitsVector) {
                    wstringstream linestream(hitlist);
                    wstring wordID;
                    wstring imp;
                    getline(linestream, wordID, L',');
                    getline(linestream, imp, L',');
                    wstring numOfHits;
                    getline(linestream, numOfHits, L',');
                    cumScore += stoi(imp) * stoi(numOfHits);
                }

                cumScore /= (double) hitsVector.size();
                docScores.emplace_back(docID, cumScore);
            }
        }

        std::sort(docScores.begin(), docScores.end(), [&](const pair<wstring, double> &a, const pair<wstring, double> &b) {
            return (a.second > b.second);
        });

        cout << "\n" << ((double) duration_cast<milliseconds>(high_resolution_clock::now() - start).count())/1000 << " seconds to fetch " << docScores.size() << " results\n";

        for (auto &item: docScores) {
            wcout << "https://stackoverflow.com/questions/" << item.first << " " << "score " << item.second << "\n";
        }
    }
}

double getCumScore(const vector<int> &queryWordIDs, unordered_map<int, pair<wstring, int>> &posWordMap, const vector<int> &hitPos) {
    double cumscore = 0;
    vector<wstring> bunchIDs;
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
    return cumscore;
}

#pragma clang diagnostic pop

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

    unordered_set<wstring> commonDocIDs;

    for (int i = 0; i < documents[0].size(); ++i){
        vector<wstring> temp;

        for (int z = 1; z < documents.size(); ++z){
            if(binary_search(documents[z].begin(), end(documents[z]), documents[0][i], [](const pair<wstring, wstring>& a, const pair<wstring, wstring>& b){
                return  a.first.size() < b.first.size() || (a.first.size() == b.first.size() && a.first < b.first);
            })){
                temp.push_back(documents[0][i].first);
            }
        }

        if (temp.size() == documents.size() - 1){commonDocIDs.insert(documents[0][i].first);}
    }

    for (auto & document : documents){
        document.erase(remove_if(document.begin(), document.end(),
                                 [&commonDocIDs](const pair<wstring, wstring> &s) { return !commonDocIDs.count(s.first); }), document.end());
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

vector<unordered_map<wstring, int>> getLineNumbers(){
    vector<unordered_map<wstring, int>> lineNums;
    wifstream lineNumberFile("../data_structures/lineNumbers.txt");
    wstring line;
    wstring wordId;
    wstring lineCount;

    auto dirIter = std::filesystem::directory_iterator(current_path().string() + "/../data_structures/barrels");
    int fileCount = count_if(
            begin(dirIter),
            end(dirIter),
            [](auto& entry) { return entry.is_regular_file(); }
    );

    for (int k = 0; k < fileCount; ++k){
        int i = 0;
        lineNums.emplace_back(unordered_map<wstring, int>());
        while(getline(lineNumberFile, line)){
            if (!line.empty()){
                wstringstream linestream(line);
                getline(linestream, wordId, L',');
                getline(linestream, lineCount, L',');
                lineNums[k][wordId] = stoi(lineCount);
                ++i;
                if (i == WORDS_IN_BARRELS){break;}
            }
        }
    }
    return lineNums;
}