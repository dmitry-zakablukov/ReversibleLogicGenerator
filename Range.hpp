#pragma once

struct Range
{
public:
    uint start;
    uint end;

    void sort();
    bool has(uint x) const;
};
