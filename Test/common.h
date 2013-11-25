#pragma once

#define END uint(-1)

word mask(uint first, ...);
word binStringToInt(string value);
string polynomialToString(word polynomial);
vector<word> makePermutationFromScheme(ReversibleLogic::Generator::Scheme scheme, uint n);