// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

// Group theory based generator class for reversible logic synthesis with additional input lines

#include "std.h"

namespace ReversibleLogic
{

Scheme GtGeneratorWithMemory::generateFast(const TruthTable& table, ostream& outputLog)
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

    outputLog << "Complexity after all optimizations: " << scheme.size() << endl;

    bool isValid = checkSchemeValidity(scheme, n, m, table);
    assert(isValid, string("Generated scheme is not valid"));

    return scheme;
}

void GtGeneratorWithMemory::detectBitCount(const TruthTable& table, uint* n, uint* m)
{
    assertd(n && m, string("Null pointer (GtGeneratorWithMemory::detectBitCount)"));

    uint size = table.size();
    if (size)
        --size;

    *n = getSignificantBitCount(size);

    TruthTable copy = table;
    sort(copy.begin(), copy.end());

    word last = copy.back();
    *m = getSignificantBitCount(last);
}

void GtGeneratorWithMemory::generateCoordinateFunction(Scheme* scheme,
    uint n, uint m, uint coord, unordered_set<word>& inputs)
{
    assertd(scheme, string("Null pointer (GtGeneratorWithMemory::generateCoordinateFunction)"));

    Scheme subScheme;
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

            subScheme.push_back(ReverseElement(n + m, targetMask, controlMask, inversionMask));
        }
        else
        {
            // no edge found, implement all inputs one by one
            for (auto& x : inputs)
            {
                word targetMask = (word)1 << (n + coord);
                word controlMask = ((word)1 << n) - 1;
                word inversionMask = x ^ controlMask;

                subScheme.push_back(ReverseElement(n + m, targetMask, controlMask, inversionMask));
            }

            // brake main loop
            break;
        }
    }

    subScheme = PostProcessor().optimize(subScheme);
    scheme->insert(scheme->end(), subScheme.cbegin(), subScheme.cend());
}

bool GtGeneratorWithMemory::checkSchemeValidity(const Scheme& scheme,
    uint n, uint m, const TruthTable& table)
{
    auto extractFunc = [=](word x)
    {
        word mask = ((word)1 << m) - 1;
        word y = (x >> n) & mask;

        return y;
    };

    bool isValid = true;

    word count = (word)1 << n;
    for (word x = 0; x < count; ++x)
    {
        word y = x;
        for (auto& element : scheme)
            y = element.getValue(y);

        if (table[x] != extractFunc(y))
        {
            isValid = false;
            break;
        }
    }

    return isValid;
}

} //namespace ReversibleLogic
