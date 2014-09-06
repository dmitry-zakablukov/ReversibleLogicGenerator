// Generator class for reversible logic synthesis with additional input lines

#include "std.hpp"

namespace ReversibleLogic
{

Scheme GeneratorWithMemory::generateFast(const TruthTable& table, ostream& outputLog)
{
    Scheme scheme;
    uint tableSize = table.size();

    uint n = 0, m = 0;
    detectBitCount(table, &n, &m);

    for (uint coord = 0; coord < m; ++coord)
    {
        unordered_set<word> inputs;
        word mask = 1 << coord;

        for (uint index = 0; index < tableSize; ++index)
        {
            if (table[index] & mask)
                inputs.insert(index);
        }

        // generate boolean edge
    }

    return scheme;
}

void GeneratorWithMemory::detectBitCount(const TruthTable& table, uint* n, uint* m)
{
    assertd(n && m, string("Null pointer (GeneratorWithMemory::detectBitCount)"));

    uint size = table.size();
    if (size)
        --size;

    *n = getSignificantBitCount(size);

    TruthTable copy = table;
    sort(copy.begin(), copy.end());

    word last = copy.back();
    *m = getSignificantBitCount(last);
}

} //namespace ReversibleLogic
