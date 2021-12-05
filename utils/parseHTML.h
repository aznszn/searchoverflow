
void parseHTML(vector<vector<wstring>>& table, vector<int>& indices){
    wregex rgx(L"<code>.*?<\\/code>");
    for(auto & i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], rgx, L"");

    rgx = L"<a.*?>";
    for(auto & i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], rgx, L"");

    rgx = L"<.{1,70}?>";
    for(auto & i : table)
        for(auto& j : indices)
            i[j] = regex_replace(i[j], rgx, L"");

}