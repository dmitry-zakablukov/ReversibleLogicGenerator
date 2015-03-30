#pragma once

typedef unordered_map<string, list<string>> Values;

#define END uint(-1)

word mask(uint first, ...);
word binStringToInt(string value);
string polynomialToString(word polynomial);
vector<word> makePermutationFromScheme(ReversibleLogic::Scheme scheme, uint n);

string trim(const string& value);
string removeQuotes(const string& value);