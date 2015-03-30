#include "std.hpp"

AssertionError::AssertionError()
    : message()
{

}

AssertionError::AssertionError(string&& message)
    : message( move(message))
{
}

const char* AssertionError::what() const
{
    return message.c_str();
}

uint countNonZeroBits(word value)
{
    uint count = 0;
    while(value)
    {
        count += value & 1;
        value >>= 1;
    }

    return count;
}

uint findPositiveBitPosition(word value, uint startPos /* = 0 */)
{
    uint position = uintUndefined;
    value >>= startPos;

    while(value)
    {
        if(value & 1)
        {
            position = startPos;
            break;
        }

        value >>= 1;
        ++startPos;
    }

    return position;
}

uint getSignificantBitCount(word value)
{
    uint count = 0;
    word mask = 1;
    
    while (mask <= value)
    {
        mask <<= 1;
        ++count;
    }

    return count;
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
