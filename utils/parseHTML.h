
void parseHTML(vector<vector<wstring>>& table, vector<int>& indices){
    const wregex code(L"<code>.*?<\\/code>");
    for(auto& i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], code, L" ");
    cout << "\ncode removed";

    const wregex links (L"https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_\\+.~#?&//=]*)");
    for(auto & i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], links, L" ");
    cout << "\nlinks removed";

    const wregex tags (L"<([a-z]|\\/){1,70}?>");
    for(auto & i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], tags, L" ");
    cout << "\nHTML tags removed";

}