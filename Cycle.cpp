#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
{
    uint elementCount = length();
    if(elements[0] == elements[elementCount - 1])
    {
        elements.resize(elementCount - 1);
        finalize();
    }
}

void Cycle::finalize()
{
    finalized = true;
}

bool Cycle::isFinal() const
{
    return finalized;
}

void Cycle::append(word element)
{
    assert(!finalized, string("Failed to append element to finalized cycle"));

    auto pos = find(elements.cbegin(), elements.cend(), element);
    if(pos == elements.cend())
    {
        elements.push_back(element);
    }
    else if(pos == elements.cbegin())
    {
        finalize();
    }
    else
    {
        ostringstream stream;
        stream << "Failed to append " << element << " to cycle";
        assert(false, stream.str());
    }
}

uint Cycle::length() const
{
    uint elementCount = elements.size();
    return elementCount;
}

bool Cycle::isEmpty() const
{
    uint elementCount = elements.size();
    return (elementCount == 0);
}

const word& Cycle::operator[]( uint index ) const
{
    return elements[index];
}

bool Cycle::operator ==(const Cycle& another) const
{
    bool result = (elements == another.elements);
    return result;
}

bool Cycle::has(const Transposition& target) const
{
    bool result = false;

    word x = target.getX();
    uint xPos = uintUndefined;

    word y = target.getY();
    uint yPos = uintUndefined;

    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        if(elements[index] == x)
        {
            xPos = index;
            if(yPos != uintUndefined)
            {
                break;
            }
        }

        if(elements[index] == y)
        {
            yPos = index;
            if(xPos != uintUndefined)
            {
                break;
            }
        }
    }

    if(xPos != uintUndefined && yPos != uintUndefined)
    {
        if(yPos < xPos)
        {
            swap(xPos, yPos);
        }

        uint delta = yPos - xPos;
        if(delta == 1 || delta == elementCount - 1)
        {
            result = true;
        }
    }

    return result;
}

Cycle::operator string() const
{
    ostringstream result;
    result << "(";

    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        result << elements[index] << ", ";
    }

    result << ")";
    return result.str();
}

word Cycle::getOutput(word input) const
{
    word output = input;
    
    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        word x = elements[index];
        if(input == x)
        {
            output = elements[modIndex(index + 1)];
        }
    }

    return output;
}

word Cycle::getOutput(word input, shared_ptr<list<Transposition>> transpositions) const
{
    word output = input;
    forcin(iter, *transpositions)
    {
        output = iter->getOutput(output);
    }

    return output;
}

uint Cycle::modIndex(uint index) const
{
    uint elementCount = length();
    uint resIndex = modIndex(index, elementCount);

    return resIndex;
}

uint Cycle::modIndex(uint index, uint mod) const
{
    uint resIndex = index;

    resIndex -= (resIndex >= mod) * mod;
    return resIndex;
}

void Cycle::multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
    bool isLeftMultiplication, vector<shared_ptr<Cycle>>* output) const
{
    unordered_set<word> visitedElements;
    unordered_set<word>::const_iterator end = visitedElements.cend();
    shared_ptr<Cycle> nextCycle(new Cycle());

    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        word x = elements[index];
        if(visitedElements.find(x) == end)
        {
            while(!nextCycle->isFinal())
            {
                word y = x;
                if(isLeftMultiplication)
                {
                    y = getOutput(y, transpositions);
                    y = getOutput(y);
                }
                else
                {
                    y = getOutput(y);
                    y = getOutput(y, transpositions);
                }

                if(nextCycle->isEmpty())
                {
                    nextCycle->append(x);
                }
                nextCycle->append(y);

                visitedElements.insert(x);
                x = y;
            }

            // skip fixed point
            uint cycleLength = nextCycle->length();
            if(cycleLength > 1)
            {
                output->push_back(nextCycle);
            }

            nextCycle = shared_ptr<Cycle>(new Cycle());
        }
    }

    assert(visitedElements.size() == elementCount,
        string("Cycle::multiplyByTranspositions() failed because not all elements processed"));

    assert(!nextCycle->length(),
        string("Cycle::multiplyByTranspositions() failed because of last cycle"));
}

void Cycle::prepareForDisjoint(unordered_map<word, uint>* frequencyMap)
{
    uint elementCount = length();
    uint stepCount = elementCount / 2;

    //// debug
    //stepCount = min(stepCount, (uint)1);

    for(uint step = 1; step <= stepCount; ++step)
    {
        uint maxIndex = elementCount;
        if(!(elementCount & 1) && step == stepCount)
        {
            // avoid pair duplicates
            maxIndex /= 2;
        }

        for(uint index = 0; index < maxIndex; ++index)
        {
            word x = elements[index];
            word y = elements[modIndex(index + step)];
            word diff = x ^ y;

            if(frequencyMap->count(diff))
            {
                (*frequencyMap)[diff] += 1;
            }
            else
            {
                (*frequencyMap)[diff] = 1;
            }
        }
    }
}

void Cycle::disjointByDiff(word diff, shared_ptr<list<Transposition>> result)
{
    getTranspositionsByDiff(elements, diff, result);
}

void Cycle::getTranspositionsByDiff(const vector<word>& input, word diff,
    shared_ptr<list<Transposition>> result)
{
    unordered_map<word, uint> elementToIndexMap;
    unordered_set<word> elementStorage;

    // remember positions of all elements and elements itself
    uint elementCount = input.size();
    for(uint index = 0; index < elementCount; ++index)
    {
        const word& element = input[index];

        elementToIndexMap[element] = index;
        elementStorage.insert(element);
    }

    // fill array of coverage by possible disjoint segments
    vector<uint> counterArray;
    counterArray.resize(elementCount);
    uint counter = 0;

    unordered_set<word>::const_iterator end = elementStorage.cend();
    for(uint index = 0; index < elementCount; ++index)
    {
        const word& x = input[index];
        word y = x ^ diff;

        if(elementStorage.find(y) != end)
        {
            uint xIndex = elementToIndexMap[x];
            uint yIndex = elementToIndexMap[y];

            if(xIndex < yIndex)
            {
                ++counter;
                counterArray[index] = counter;
            }
            else
            {
                counterArray[index] = counter;
                --counter;
            }
        }
        else
        {
            counterArray[index] = counter;
        }
    }

    // find best disjoint indices
    uint bestLeftIndex = uintUndefined;
    uint bestRightIndex = uintUndefined;
    uint minSum = uintUndefined;

    for(uint index = 0; index < elementCount; ++index)
    {
        const word& x = input[index];
        word y = x ^ diff;

        if(elementStorage.find(y) != end)
        {
            uint xIndex = elementToIndexMap[x];
            uint yIndex = elementToIndexMap[y];

            if(xIndex < yIndex)
            {
                uint sum = counterArray[xIndex] + counterArray[yIndex];
                if(sum < minSum)
                {
                    minSum = sum;
                    bestLeftIndex = xIndex;
                    bestRightIndex = yIndex;
                }
            }
        }
    }

    if(minSum != uintUndefined)
    {
        const word& x = input[bestLeftIndex];
        word y = x ^ diff;

        result->push_back(Transposition(x, y));
        getTranspositionsByDiff(input, diff, bestLeftIndex, bestRightIndex, result);
    }
}

void Cycle::getTranspositionsByDiff(const vector<word>& input, word diff,
    uint xIndex, uint yIndex, shared_ptr<list<Transposition>> result)
{
    uint distance = yIndex - xIndex;

    // 1) make middle part vector
    if(distance > 2)
    {
        vector<word> middlePart;
        middlePart.resize(distance - 1);
        memcpy(middlePart.data(), input.data() + xIndex + 1, (distance - 1) * sizeof(word));

        getTranspositionsByDiff(middlePart, diff, result);
    }

    // 2) make rest part vector
    uint elementCount = input.size();
    if(distance + 2 < elementCount) //unsigned, so this is just condition (elementCount - (distance + 1) > 1)
    {
        vector<word> restPart;
        restPart.resize(elementCount - distance - 1);

        memcpy(restPart.data(), input.data(), xIndex * sizeof(word));
        memcpy(restPart.data() + xIndex, input.data() + yIndex + 1, (elementCount - yIndex - 1) * sizeof(word));

        getTranspositionsByDiff(restPart, diff, result);
    }
}

uint Cycle::getDistancesSum() const
{
    uint sum = 0;

    uint elementCount = length();
    uint stepCount = elementCount / 2;

    for(uint step = 1; step <= stepCount; ++step)
    {
        uint maxIndex = elementCount;
        if(!(elementCount & 1) && step == stepCount)
        {
            // avoid pair duplicates
            maxIndex /= 2;
        }

        for(uint index = 0; index < maxIndex; ++index)
        {
            word x = elements[index];
            word y = elements[modIndex(index + step)];
            word diff = x ^ y;

            sum += countNonZeroBits(diff);
        }
    }

    return sum;
}

}   // namespace ReversibleLogic

