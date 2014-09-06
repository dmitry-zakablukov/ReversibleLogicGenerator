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
