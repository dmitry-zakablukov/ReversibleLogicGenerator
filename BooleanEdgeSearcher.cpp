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

#include "std.hpp"

//////////////////////////////////////////////////////////////////////////
// struct BooleanEdge

BooleanEdge::BooleanEdge(uint n)
    : n(n)
    , baseValue(wordUndefined)
    , starsMask(wordUndefined)
    , full(false)
    , coveredTranspositionCount(0)
{
}

bool BooleanEdge::isFull() const
{
    return full;
}

bool BooleanEdge::isValid() const
{
    return full || (baseValue != wordUndefined && starsMask != wordUndefined);
}

word BooleanEdge::getCapacity() const
{
    word edgeCapacity = 0;
    if(starsMask != wordUndefined)
    {
        uint starsCount = countNonZeroBits(starsMask);
        edgeCapacity = (word)1 << starsCount;
    }

    return edgeCapacity;
}

word BooleanEdge::getBaseMask() const
{
    word fullMask = ((word)1 << n) - 1;
    word baseMask = fullMask & ~starsMask;

    return baseMask;
}

word BooleanEdge::getBaseValue() const
{
    word baseMask = getBaseMask();
    return baseValue & baseMask;
}

bool BooleanEdge::has(word x) const
{
    return (x & getBaseMask()) == getBaseValue();
}

//////////////////////////////////////////////////////////////////////////
// class BooleanEdgeSearcher

BooleanEdgeSearcher::BooleanEdgeSearcher(
    shared_ptr<list<ReversibleLogic::Transposition>> input,
    uint n, word initialMask)
    : inputSet()
    , n(n)
    , initialMask(initialMask)
    , frequencyTable()
{
    for (auto& transp : *input)
    {
        inputSet.insert(transp.getX());
        inputSet.insert(transp.getY());
    }

    validateInputSettings();
}

BooleanEdgeSearcher::BooleanEdgeSearcher(const unordered_set<word>& inputs, uint n)
    : inputSet(inputs)
    , n(n)
    , initialMask(0)
    , frequencyTable()
{
    validateInputSettings();
}

void BooleanEdgeSearcher::setExplicitEdgeFlag(bool value)
{
    explicitEdgeFlag = value;
}

void BooleanEdgeSearcher::validateInputSettings()
{
    assertd(inputSet.size(), string("Empty input set"));
    assertd(n != uintUndefined, string("n is not defined"));
    assertd(initialMask <= (word)((1 << n) - 1), string("Initial mask is not valid"));
}

BooleanEdge BooleanEdgeSearcher::findEdge()
{
    BooleanEdge edge(n);
    uint inputLength = inputSet.size();

    if(inputLength == ((uint)1 << n))
    {
        // this is full boolean cube
        edge.starsMask = initialMask;
        edge.full = true;
    }
    else
    {
        // find upper bound for edge dimension
        uint maxEdgeDimension = findMaxEdgeDimension(inputLength);
        uint minEdgeDimension = min((uint)1, countNonZeroBits(initialMask));

        // find maximum edge
        while(minEdgeDimension <= maxEdgeDimension)
        {
            findEdge(&edge, initialMask, maxEdgeDimension - minEdgeDimension, 0);
            if(edge.isValid())
                break;

            --maxEdgeDimension;
        }
    }

    return edge;
}

uint BooleanEdgeSearcher::findMaxEdgeDimension(uint length)
{
    uint maxEdgeDimesion = 0;
    while(length)
    {
        ++maxEdgeDimesion;
        length >>= 1;
    }

    return maxEdgeDimesion;
}

void BooleanEdgeSearcher::findEdge(BooleanEdge* bestEdge, word edgeMask,
    uint restPositionCount, uint startPos)
{
    if(restPositionCount)
    {
        while(startPos <= n - restPositionCount)
        {
            word mask = 1 << startPos;
            if(!(edgeMask & mask))
                findEdge(bestEdge, mask ^ edgeMask, restPositionCount - 1, startPos + 1);

            ++startPos;
        }
    }
    else
    {
        BooleanEdge edge(n);
        edge.starsMask = edgeMask;

        bool isValid = checkEdge(&edge);
        if(isValid && edge.coveredTranspositionCount > bestEdge->coveredTranspositionCount)
            *bestEdge = edge;
    }
}

bool BooleanEdgeSearcher::checkEdge(BooleanEdge* edge)
{
    bool result = false;

    // prepare frequency table
    uint maxEntriesCount = (uint)1 << n;
    frequencyTable.resize(maxEntriesCount);
    memset(&frequencyTable[0], 0, maxEntriesCount * sizeof(frequencyTable[0]));

    // calculate edge capacity
    word edgeCapacity = edge->getCapacity();

    // calculate frequency of all entries according to the edge
    word fullMask = ((word)1 << n) - 1;
    word mask = (fullMask ^ edge->starsMask) & fullMask;

    for (auto& x : inputSet)
    {
        assertd(x < maxEntriesCount, string("Invalid x value in input set"));

        word entry = x & mask;
        uint& counter = frequencyTable[entry];
        ++counter;

        if (counter == edgeCapacity)
        {
            // edge generates subset in input set
            edge->baseValue = entry;
            edge->coveredTranspositionCount = edgeCapacity;

            result = true;
            break;
        }
    }

    if (!result && !explicitEdgeFlag)
    {
        uint bestIndex = 0;
        uint maxCounter = 0;

        uint index = 0;
        if (edge->starsMask == ((1 << n) - 1))
            index = 1;

        for (; index < maxEntriesCount; ++index)
        {
            uint counter = frequencyTable[index];
            if (counter > maxCounter)
            {
                maxCounter = counter;
                bestIndex = index;
            }
        }

        if (maxCounter * 2 > edgeCapacity)
        {
            edge->baseValue = bestIndex;
            edge->coveredTranspositionCount = maxCounter;
            result = true;
        }
    }

    return result;
}

// static
shared_ptr<list<ReversibleLogic::Transposition>> BooleanEdgeSearcher::filterTranspositionsByEdge(BooleanEdge edge, uint n,
    shared_ptr<list<ReversibleLogic::Transposition>> transpositions)
{
    using namespace ReversibleLogic;

    word baseValue = edge.getBaseValue();
    word baseMask  = edge.getBaseMask();

    shared_ptr<list<Transposition>> filteredResult(new list<Transposition>);
    for (auto& transp : *transpositions)
    {
        word x = transp.getX();

        word value = x & baseMask;
        if(value == baseValue)
            filteredResult->push_back(transp);
    }

    return filteredResult;
}

shared_ptr<list<ReversibleLogic::Transposition>> BooleanEdgeSearcher::getEdgeSubset(BooleanEdge edge, uint n)
{
    using namespace ReversibleLogic;

    word starsMask = edge.starsMask;
    uint starCount = countNonZeroBits(starsMask);

    vector<word> stars;
    stars.reserve(starCount);

    uint pos = findPositiveBitPosition(starsMask, 0);
    while (pos != uintUndefined)
    {
        stars.push_back((word)1 << pos);
        pos = findPositiveBitPosition(starsMask, pos + 1);
    }
    assertd(stars.size() == starCount, string("Not all star masks found"));

    word totalCount = (word)1 << starCount;
    unordered_set<word> visitedElements;
    word baseValue = edge.getBaseValue();

    shared_ptr<list<Transposition>> subset(new list<Transposition>);
    for(word index = 0; index < totalCount; ++index)
    {
        word x = baseValue;
        for (uint starPos = 0; starPos < starCount; ++starPos)
        {
            word mask = (word)1 << starPos;
            x ^= ((index & mask) != 0) * stars[starPos];
        }

        word y = x ^ initialMask;
        if(visitedElements.find(x) == visitedElements.cend())
        {
            subset->push_back(Transposition(x, y));

            visitedElements.insert(x);
            visitedElements.insert(y);
        }
    }

    return subset;
}

shared_ptr<unordered_set<word>> BooleanEdgeSearcher::getEdgeSet(BooleanEdge edge)
{
    shared_ptr<unordered_set<word>> edgeSet(new unordered_set<word>);

    word starsMask = edge.starsMask;
    uint starCount = countNonZeroBits(starsMask);

    vector<word> stars;
    stars.reserve(starCount);

    uint pos = findPositiveBitPosition(starsMask, 0);
    while (pos != uintUndefined)
    {
        stars.push_back((word)1 << pos);
        pos = findPositiveBitPosition(starsMask, pos + 1);
    }
    assertd(stars.size() == starCount, string("Not all star masks found"));

    word totalCount = (word)1 << starCount;
    word baseValue = edge.getBaseValue();

    for (word index = 0; index < totalCount; ++index)
    {
        word x = baseValue;
        for (uint starPos = 0; starPos < starCount; ++starPos)
        {
            word mask = (word)1 << starPos;
            x ^= ((index & mask) != 0) * stars[starPos];
        }

        edgeSet->insert(x);
    }

    return edgeSet;
}
