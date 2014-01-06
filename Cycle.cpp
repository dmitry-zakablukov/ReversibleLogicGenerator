#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , disjointTransp()
    , disjointIndex(0)
    , firstTranspositions()
    , restElements()
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , disjointTransp()
    , disjointIndex(0)
    , firstTranspositions()
    , restElements()
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

Transposition Cycle::getTranspositionByPosition(uint index) const
{
    Transposition transp;
    
    uint elementCount = length();
    if(index < elementCount)
    {
        word x = elements[index];
        word y = 0;

        if(index == elementCount - 1)
        {
            y = elements[0];
        }
        else
        {
            y = elements[index + 1];
        }

        transp.setX(x);
        transp.setY(y);
    }

    return transp;
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

bool Cycle::remove(const Transposition& target)
{
    assert(found(*firstTranspositions, target),
        string("Removing non-existent transposition from cycle"));

    firstTranspositions->remove(target);
    bool hasTranspositions = !!firstTranspositions->size();

    if(!hasTranspositions)
    {
        elements = restElements;
        distnancesSum = newDistancesSum;
    }

    return hasTranspositions;
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

void Cycle::calculateDistancesSum()
{
    distnancesSum = 0;

    // we assume, that element count > 2
    uint elementCount = length();
    for(uint index = 0; index < elementCount; ++index)
    {
        distnancesSum += countNonZeroBits(elements[index] ^ elements[modIndex(index + 1)]);
    }

    newDistancesSum = distnancesSum;
}

shared_ptr<list<Transposition>> Cycle::getBestTranspositionsForDisjoint()
{
    firstTranspositions = 0;

    uint elementCount = length();
    if(elementCount == 2)
    {
        word x = elements[0];
        word y = elements[1];

        firstTranspositions = shared_ptr<list<Transposition>>(new list<Transposition>);
        firstTranspositions->push_back(Transposition(x, y));
    }
    else if(elementCount > 2)
    {
        calculateDistancesSum();

        unordered_map<word, uint> sums;
        uint minSum = uintUndefined;
        word bestFirstDiff = 0;

        for(uint index = 0; index < elementCount; ++index)
        {
            uint sum = distnancesSum;

            word dxy = elements[index] ^ elements[modIndex(index + 1)];
            word dxz = elements[index] ^ elements[modIndex(index + 2)];
            word dyz = elements[modIndex(index + 1)] ^ elements[modIndex(index + 2)];

            sum += countNonZeroBits(dxz);
            sum -= countNonZeroBits(dyz);

            if(!sums.count(dxy))
            {
                sums[dxy] = 0;
            }

            sums[dxy] += sum;

            if(sum < minSum)
            {
                minSum = sum;
                bestFirstDiff = dxy;
            }
        }

        // bugbug: comment this for testing the best strategy
        //bestFirstDiff = findBestDiff(sums);

        firstTranspositions = getTranspositionsByDiff(bestFirstDiff);
    }

    return firstTranspositions;
}

word Cycle::findBestDiff( unordered_map<word, uint> sums )
{
    // sort keys
    typedef struct 
    {
        word diff;
        uint sum;
    } TempSortStruct;

    vector<TempSortStruct> keys(sums.size());
    uint index = 0;

    forcin(iter, sums)
    {
        TempSortStruct key;

        key.diff = iter->first;
        key.sum  = iter->second;

        keys[index] = key;
        ++index;
    }

    auto sortFunc = [](const TempSortStruct& leftKey, const TempSortStruct& rightKey) -> bool
    {        
        bool isLess = (leftKey.sum < rightKey.sum);
        return isLess;
    };

    sort(keys.begin(), keys.end(), sortFunc);
    return keys[0].diff;
}

void Cycle::removeElement(vector<word>* target, uint index)
{
    if(target)
    {
        uint elementCount = target->size();
        assert(index < elementCount, string("Removing non-existent element from vector"));

        word* ptr = target->data();
        if(index < elementCount - 1)
        {
            memcpy(ptr + index, ptr + index + 1,
                (elementCount - index - 1) * sizeof(word));
        }

        target->resize(elementCount - 1);
    }
}

shared_ptr<list<Transposition>> Cycle::getTranspositionsByDiff(word diff)
{
    shared_ptr<list<Transposition>> result(new list<Transposition>);

    uint elementCount = length();
    newDistancesSum = distnancesSum;

    restElements = elements;
    uint restIndex = 0;

    for(uint index = 0; index < elementCount; ++index)
    {
        word x = elements[index];
        word y = elements[modIndex(index + 1)];
        word dxy = x ^ y;

        if(dxy == diff)
        {
            // save found transposition to result
            result->push_back(Transposition(x, y));
        }
        else
        {
            ++restIndex;
        }
    }

    return result;
}

shared_ptr<Cycle> Cycle::multiplyByTranspositions(
    shared_ptr<list<Transposition>> transpositions,
    bool isLeftMultiplication) const
{
    // remember all elements in transpositions to remove
    // and difference for them
    word diff = transpositions->front().getDiff();

    unordered_set<word> targetElements;
    forcin(transp, *transpositions)
    {
        targetElements.insert(transp->getX());
        targetElements.insert(transp->getY());
    }

    return multiplyByTranspositions(targetElements, diff, isLeftMultiplication);
}

shared_ptr<Cycle> Cycle::multiplyByTranspositions(
    const unordered_set<word>& targetElements, word diff, bool isLeftMultiplication) const
{
    // 1) create new vector of elements
    uint elementCount = length();

    vector<word> resultElements;
    resultElements.reserve(elementCount);

    // 2) fill this vector
    bool copyNeeded = true;
    if(elementCount == 2 && targetElements.count(elements[0]))
    {
        // whole cycle need to be removed
        copyNeeded = false;
    }

    if(copyNeeded)
    {
        for(uint index = 0; index < elementCount; ++index)
        {
            const word& first  = elements[index];
            const word& second = elements[modIndex(index + 1)];

            if(targetElements.count(first))
            {
                if(targetElements.count(second) && ((first ^ second) == diff))
                {
                    if(isLeftMultiplication)
                    {
                        resultElements.push_back(first);
                    }
                    else
                    {
                        resultElements.push_back(second);
                    }

                    // skip second element
                    ++index;
                }
            }
            else
            {
                resultElements.push_back(first);
            }
        }
    }

    // 3) create result cycle
    shared_ptr<Cycle> resultCycle = 0;
    if(resultElements.size())
    {
        resultCycle = shared_ptr<Cycle>(new Cycle(move(resultElements)));
    }

    return resultCycle;
}

}   // namespace ReversibleLogic

