#include "std.hpp"


void Range::sort()
{
    if (end < start)
    {
        end   ^= start;
        start ^= end;
        end   ^= start;
    }
}

bool Range::has(uint x) const
{
    return x >= start && x <= end;
}
