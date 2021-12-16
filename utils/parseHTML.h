//
// Created by HP on 08/12/2021.
//

#ifndef UNTITLED_PARSEHTML_H
#define UNTITLED_PARSEHTML_H

#define code_rgx L"<code>.*?<\\/code>"
#define link_rgx L"https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_\\+.~#?&//=]*)"
#define tag_rgx L"<([a-z]|\\/){1,70}?>"

#include <regex>

void parseHTML(vector<vector<wstring>>& table, int i){
    const wregex code(code_rgx);
    const wregex links(link_rgx);
    const wregex tags(tag_rgx);

    for(auto & row : table)
        row[i] = regex_replace(
                regex_replace(
                        regex_replace(row[i], code, L" "),
                        links, L" "),
                tags, L" ");

}

#endif //UNTITLED_PARSEHTML_H
