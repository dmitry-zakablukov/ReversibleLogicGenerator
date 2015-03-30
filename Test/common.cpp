#include "std.hpp"

word mask(uint first, ...)
{
    word result = (word)1 << first;

    va_list list;
    va_start(list, first);
    uint temp = va_arg(list, uint);

    while(temp != uintUndefined)
    {
        result += (word)1 << temp;
        temp = va_arg(list, uint);
    }

    return result;
}

word binStringToInt(string value)
{
    word number = 0;
    for (auto& symbol : value)
    {
        number <<= 1;
        if(symbol == '1')
            number |= 1;
    }

    return number;
}

string polynomialToString(word polynomial)
{
    ostringstream repres;

    uint index = 0;
    word mask = 1;

    while (mask <= polynomial)
    {
        if (polynomial & mask)
        {
            if (index)
            {
                if (polynomial & 1)
                    repres << " +";

                if (index == 1)
                    repres << " x";
                else
                    repres << " " << index << "^x";
            }
            else
                repres << "1";
        }

        mask <<= 1;
        ++index;
    }

    string result = repres.str();
    reverse(result.begin(), result.end());
    return result;
}

vector<word> makePermutationFromScheme(ReversibleLogic::Scheme scheme,
    uint n)
{
    using namespace ReversibleLogic;

    word transformCount = 1 << n;
    vector<word> table(transformCount);

    for(word x = 0; x < transformCount; ++x)
    {
        word y = x;
        for(Scheme::const_iterator iter = scheme.begin();
            iter != scheme.end(); ++iter)
        {
            y = (*iter).getValue(y);
            assertd((y < transformCount), string("Wrong transform"));
        }

        table[x] = y;
    }

    return table;
}

string trim(const string& value)
{
    uint size = value.size();

    // find first non-space character
    string::const_iterator first = value.cbegin();
    while (isspace(*first))
        ++first;

    // find last non-space character
    string::const_reverse_iterator lastReverse = value.crbegin();
    while (isspace(*lastReverse))
        ++lastReverse;

    string::const_iterator last = lastReverse.base();

    string result;
    result.reserve(size);

    while (first != last)
        result.push_back(*first++);

    return result;
}

string removeQuotes(const string& value)
{
    if (value.size() == 0)
        return value;

    uint start = 0;
    uint count = value.size();

    if (value.front() == '"')
    {
        ++start;
        --count;
    }

    if (value.back() == '"')
        --count;

    string result = value.substr(start, count);
    return result;
}
