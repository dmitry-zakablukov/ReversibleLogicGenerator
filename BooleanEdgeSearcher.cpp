#include "std.hpp"

//////////////////////////////////////////////////////////////////////////
// struct BooleanEdge

BooleanEdge::BooleanEdge()
    : baseValue(wordUndefined)
    , starsMask(wordUndefined)
    , full(false)
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
        edgeCapacity = (uint)1 << starsCount;
    }

    return edgeCapacity;
}

word BooleanEdge::getBaseMask(uint n) const
{
    word fullMask = ((word)1 << n) - 1;
    word baseMask = fullMask & ~starsMask;

    return baseMask;
}

word BooleanEdge::getBaseValue(uint n) const
{
    word baseMask = getBaseMask(n);
    return baseValue & baseMask;
}

//////////////////////////////////////////////////////////////////////////
// class BooleanEdgeSearcher

BooleanEdgeSearcher::BooleanEdgeSearcher(
    shared_ptr<list<ReversibleLogic::Transposition>> inputSet,
    uint n, word initialMask)
    : inputSet(inputSet)
    , n(n)
    , initialMask(initialMask)
    , frequencyTable()
{
    validateInputSettings();
}

BooleanEdgeSearcher::~BooleanEdgeSearcher()
{
    inputSet = 0;
}

void BooleanEdgeSearcher::validateInputSettings()
{
    assert(inputSet, string("Empty input set"));
    assert(n != uintUndefined, string("n is not defined"));
    assert(initialMask <= (word)((1 << n) - 1), string("Initial mask is not valid"));
}

BooleanEdge BooleanEdgeSearcher::findEdge()
{
    BooleanEdge edge;
    uint inputLength = inputSet->size();

    if(inputLength == ((uint)1 << (n - 1)))
    {
        // this is full boolean cube
        edge.starsMask = initialMask;
        edge.full = true;
    }
    else
    {
        // find upper bound for edge dimension
        uint maxEdgeDimension = findMaxEdgeDimension(inputLength);
        uint minEdgeDimension = countNonZeroBits(initialMask);

        // find maximum edge
        while(minEdgeDimension <= maxEdgeDimension)
        {
            edge = findEdge(initialMask, maxEdgeDimension - minEdgeDimension, 0);
            if(edge.isValid())
            {
                break;
            }

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

BooleanEdge BooleanEdgeSearcher::findEdge(word edgeMask, uint restPositionCount, uint startPos)
{
    BooleanEdge edge;
    if(restPositionCount)
    {
        while(startPos <= n - restPositionCount)
        {
            word mask = 1 << startPos;
            if(!(edgeMask & mask))
            {
                edge = findEdge(mask ^ edgeMask, restPositionCount - 1, startPos + 1);
                if(edge.isValid())
                {
                    break;
                }
            }

            ++startPos;
        }
    }
    else
    {
        edge.starsMask = edgeMask;
        if(!checkEdge(&edge))
        {
            edge.baseValue = uintUndefined;
            edge.starsMask = uintUndefined;
        }
    }

    return edge;
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
    word mask = ~(edge->starsMask);
    forcin(iter, *inputSet)
    {
        const ReversibleLogic::Transposition& transp = *iter;
        word x = transp.getX();
        word y = transp.getY();

        assert(x < maxEntriesCount, string("Invalid x value in input set"));
        assert(y < maxEntriesCount, string("Invalid y value in input set"));

        // process x
        {
            word entry = x & mask;
            uint& counter = frequencyTable[entry];
            ++counter;

            if(counter == edgeCapacity)
            {
                // edge generates subset in input set
                edge->baseValue = entry;
                result = true;
                break;
            }
        }

        // process y
        {
            word entry = y & mask;
            uint& counter = frequencyTable[entry];
            ++counter;

            if(counter == edgeCapacity)
            {
                // edge generates subset in input set
                edge->baseValue = entry;
                result = true;
                break;
            }
        }
    }

    return result;
}

shared_ptr<list<ReversibleLogic::Transposition>>
BooleanEdgeSearcher::getEdgeSubset(BooleanEdge edge, uint n)
{
    validateInputSettings();
    
    return getEdgeSubset(edge, n, inputSet);
}

shared_ptr<list<ReversibleLogic::Transposition>> BooleanEdgeSearcher::getEdgeSubset(BooleanEdge edge, uint n,
    shared_ptr<list<ReversibleLogic::Transposition>> transpositions)
{
    using namespace ReversibleLogic;

    word baseValue = edge.getBaseValue(n);
    word baseMask  = edge.getBaseMask(n);

    shared_ptr<list<Transposition>> subset(new list<Transposition>);
    forcin(iter, *transpositions)
    {
        const Transposition& transp = *iter;
        word x = transp.getX();

        word value = x & baseMask;
        if(value == baseValue)
        {
            subset->push_back(transp);
        }
    }

    return subset;
}
