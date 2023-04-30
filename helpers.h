//
// Created by Monish on 29-04-2023.
//
#include <regex>
#include <vector>

#ifndef STATEFULFAAS_HELPERS_H
#define STATEFULFAAS_HELPERS_H

vector<string> split(const string& s) {
    vector<string> elems;

    regex re("\\s+");

    sregex_token_iterator iter(s.begin(), s.end(), re, -1);
    sregex_token_iterator end;

    while (iter != end) {
        if (iter->length()) {
            elems.push_back(*iter);
        }
        ++iter;
    }

    return elems;
}

#endif //STATEFULFAAS_HELPERS_H
