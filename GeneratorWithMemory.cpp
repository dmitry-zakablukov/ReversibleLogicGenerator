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

        generateCoordinateFunction(&scheme, n, m, coord, inputs);
    }

    scheme = PostProcessor().optimize(scheme);
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

void GeneratorWithMemory::generateCoordinateFunction(Scheme* scheme,
    uint n, uint m, uint coord, unordered_set<word>& inputs)
{
    assertd(scheme, string("Null pointer (GeneratorWithMemory::generateCoordinateFunction)"));

    while (inputs.size())
    {
        BooleanEdgeSearcher edgeSearcher(inputs, n);
        //edgeSearcher.setExplicitEdgeFlag(true);

        BooleanEdge edge = edgeSearcher.findEdge();
        if (edge.isValid() && edge.getCapacity() > 1)
        {
            // reduce input set by edge
            auto edgeSet = edgeSearcher.getEdgeSet(edge);
            for (auto& x : *edgeSet)
            {
                if (inputs.find(x) != inputs.cend())
                    inputs.erase(x);
                else
                    inputs.insert(x);
            }

            // insert new element to scheme
            word targetMask  = (word)1 << (n + coord);
            word controlMask = edge.getBaseMask();
            word inversionMask = edge.getBaseValue() ^ controlMask;

            scheme->push_back(ReverseElement(n + m, targetMask, controlMask, inversionMask));
        }
        else
        {
            // no edge found, implement all inputs one by one
            for (auto& x : inputs)
            {
                word targetMask = (word)1 << (n + coord);
                word controlMask = ((word)1 << n) - 1;
                word inversionMask = x ^ controlMask;

                scheme->push_back(ReverseElement(n + m, targetMask, controlMask, inversionMask));
            }

            // brake main loop
            break;
        }
    }
}

} //namespace ReversibleLogic
