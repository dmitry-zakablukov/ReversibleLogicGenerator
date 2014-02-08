#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
    , previousMultiplicationType(mtNone)
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
    , previousMultiplicationType(mtNone)
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

word Cycle::getOutput(word input, const list<shared_ptr<Cycle>>& cycles) const
{
    word output = input;
    forcin(iter, cycles)
    {
        const Cycle& cycle = **iter;
        output = cycle.getOutput(output);
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

list<shared_ptr<Cycle>> Cycle::multiplyByTranspositions(
    const unordered_set<word>& targetElements, bool isLeftMultiplication,
    bool* hasNewCycles)
{
    *hasNewCycles = false;
    list<shared_ptr<Cycle>> restCycles;

    DisjointResult* disjointResult = (isLeftMultiplication ? &leftDisjointResult : &rightDisjointResult);

    bool multipliedFlag = false;
    shared_ptr<list<Transposition>> transpositions = disjointResult->transpositions;

    for(auto iter = transpositions->begin(); iter != transpositions->end(); )
    {
        Transposition& transp = *iter;
        word x = transp.getX();

        if(targetElements.count(x))
        {
            iter = transpositions->erase(iter);
            multipliedFlag = true;
        }
        else
        {
            ++iter;
        }
    }

    if(multipliedFlag)
    {
        *hasNewCycles = true;
        restCycles = getRestCycles(isLeftMultiplication);
    }

    return restCycles;
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

list<shared_ptr<Cycle>> Cycle::getRestCycles(bool isLeftMultiplication)
{
    DisjointResult* disjointResult = (isLeftMultiplication ? &leftDisjointResult : &rightDisjointResult);
    shared_ptr<list<Transposition>> transpositions = disjointResult->transpositions;
    const list<shared_ptr<Cycle>> restCycles = disjointResult->restCycles;

    list<shared_ptr<Cycle>> newCycles;
    getCyclesAfterMultiplication(isLeftMultiplication, transpositions, restCycles, &newCycles);

    return newCycles;
}

void Cycle::getCyclesAfterMultiplication(bool isLeftMultiplication,
    shared_ptr<list<Transposition>> transpositions,
    const list<shared_ptr<Cycle>>& cycles, list<shared_ptr<Cycle>>* result)
{
    if(!transpositions->size())
    {
        *result = cycles;
    }
    else
    {
        // multiply transpositions on rest cycles

        unordered_set<word> visitedElements;
        shared_ptr<Cycle> nextCycle(new Cycle());

        uint elementCount = length();
        for(uint index = 0; index < elementCount; ++index)
        {
            word x = elements[index];
            if(!visitedElements.count(x))
            {
                while(!nextCycle->isFinal())
                {
                    word y = x;
                    if(isLeftMultiplication)
                    {
                        y = getOutput(y, transpositions);
                        y = getOutput(y, cycles);
                    }
                    else
                    {
                        y = getOutput(y, cycles);
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

                uint cycleLength = nextCycle->length();

                // skip fixed point
                if(cycleLength > 1)
                {
                    result->push_back(nextCycle);
                }

                nextCycle = shared_ptr<Cycle>(new Cycle());
            }
        }

        assert(visitedElements.size() == elementCount,
            string("Cycle::getCyclesAfterMultiplication() failed because not all elements processed"));

        assert(!nextCycle->length(),
            string("Cycle::getCyclesAfterMultiplication() failed because of last cycle"));
    }
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

    // find first pair with desired Hamming distance
    unordered_set<word>::const_iterator end = elementStorage.cend(); 
    for(uint index = 0; index < elementCount; ++index)
    {
        const word& x = input[index];
        word y = x ^ diff;

        if(elementStorage.find(y) != end)
        {
            result->push_back(Transposition(x, y));

            uint xIndex = elementToIndexMap[x];
            uint yIndex = elementToIndexMap[y];
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
            if(elementCount - distance - 1 > 2)
            {
                vector<word> restPart;
                restPart.resize(elementCount - distance - 1);

                memcpy(restPart.data(), input.data(), xIndex * sizeof(word));
                memcpy(restPart.data() + xIndex, input.data() + yIndex + 1, (elementCount - yIndex - 1) * sizeof(word));

                getTranspositionsByDiff(restPart, diff, result);
            }

            break;
        }
    }
}

void Cycle::completeDisjoint(bool isLeftMultiplication,
    shared_ptr<list<Transposition>> result)
{
    assert(result && result->size() == 1,
        string("Cycle::completeDisjoint() called with wrong result"));

    const Transposition& transp = result->front();

    DisjointResult* disjointResult = (isLeftMultiplication ? &leftDisjointResult : &rightDisjointResult);
    list<shared_ptr<Cycle>>& restCycles = disjointResult->restCycles;

    assert(restCycles.size() == 1, string("Cycle::completeDisjoint() one cycle expected but failed"));
    Cycle& cycle = *(restCycles.front());

    uint complexity = uintUndefined;
    Transposition secondTransp = cycle.findBestPair(transp, &complexity);

    // push transposition to result
    result->push_back(secondTransp);

    // push transposition to cycle transpositions
    disjointResult->transpositions->push_back(secondTransp);

    // transform rest cycles to exclude
    shared_ptr<list<Transposition>> shortTransposition(new list<Transposition>);
    shortTransposition->push_back(secondTransp);

    list<shared_ptr<Cycle>> newCycles;
    getCyclesAfterMultiplication(isLeftMultiplication, shortTransposition,
        restCycles, &newCycles);

    disjointResult->restCycles = newCycles;
}

Transposition Cycle::findBestPair(const Transposition& transp, uint* complexity)
{
    word targetX = transp.getX();
    word targetY = transp.getY();

    uint elementCount = length();
    uint stepCount = elementCount / 2;

    uint minComplexity = uintUndefined;
    Transposition secondTransp;

    // debug
    stepCount = 1;
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

            if(elementCount > 3 && transp.has(x) || transp.has(y))
            {
                // we search independent transpositions pair if possible
                // for cycle with 3 elements only pair of dependent transpositions can be obtained
                continue;
            }

            Transposition candidate(x, y);
            uint complexity =
                TransposPair(transp, candidate).getEstimateImplComplexity();
            
            if(complexity < minComplexity)
            {
                minComplexity = complexity;
                secondTransp = candidate;
            }
        }
    }

    return secondTransp;
}

}   // namespace ReversibleLogic

