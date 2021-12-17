//
// Created by HP on 16/12/2021.
//

#include "stemminglib/english_stem.h"
#include <sstream>

using namespace std;

vector<wstring> readNext(wifstream& file);
vector<vector<wstring>> fetch_table(wifstream& file);
vector<wstring> getAndParseQuery();
unordered_map<wstring, int> getLexicon();
unordered_map<wstring, vector<wstring>> getDocsInfo();

int main(){
    unordered_map<wstring, vector<wstring>> docs_info = getDocsInfo();
    unordered_map<wstring, int> lexicon = getLexicon();
    vector<wstring> words = getAndParseQuery();


    vector<int> queryWordIDs;
    for(auto& w : words){
        if(lexicon.count(w)){
            queryWordIDs.push_back(lexicon[w]);
        }
    }

    vector<vector<wstring>> docs;
    for(auto& ID : queryWordIDs){
        wifstream barrel("../data_structures/f_index/" + to_string(ID/500) + ".txt");
        wstring docID;
        wstring id_in_barrel;
        vector<wstring> wordID_docs;

        while(getline(barrel, docID, L',')){
            getline(barrel, id_in_barrel, L',');
            if(stoi(id_in_barrel) == (ID%500)){
                wordID_docs.push_back(docID);
            }
            if(stoi(id_in_barrel) > ID){
                break;
            }
            getline(barrel, docID);
        }
        docs.push_back(wordID_docs);
    }


    for(auto& item: docs){
        sort(item.begin(), item.end());
    }

    int max =  0;
    for(auto& doc : docs){
        if(doc.size() > max)
            max = doc.size();
    }

    vector<wstring> intersection(max);

    if (queryWordIDs.size() > 1) {
        set_intersection(docs[0].begin(), docs[0].end(), docs[1].begin(), docs[1].end(), intersection.begin());
        for (int i = 2; i < docs.size(); ++i) {
            vector<wstring> temp(max);
            set_intersection(intersection.begin(), intersection.end(), docs[i].begin(), docs[i].end(), temp.begin());

            temp.erase(remove_if(temp.begin(), temp.end(),
                                         [](const wstring& s)
                                         { return s.empty(); }), temp.end());

            intersection = temp;
            temp.clear();
        }

        intersection.erase(remove_if(intersection.begin(), intersection.end(),
                                 [](const wstring& s)
                                 { return s.empty(); }), intersection.end());

        sort(intersection.begin(), intersection.end(), [&docs_info](const wstring& a, const wstring& b) {
            if (a.empty() || b.empty())
                return false;
            //wcout << a << L" " <<  b << endl;
            return stod(docs_info[a][1]) > stod(docs_info[b][1]);
        });
    }

    else if(!queryWordIDs.empty()){
        intersection = docs[0];
    }

    else {
        cout << "Words do not exist in the Lexicon" << endl;
    }


    for(auto& document : intersection){
        if(!document.empty())
            wcout << "https://stackoverflow.com/questions/" << document << "\n";
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