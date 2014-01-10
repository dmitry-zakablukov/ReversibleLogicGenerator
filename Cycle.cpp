#include "std.hpp"

namespace ReversibleLogic
{

Cycle::Cycle()
    : elements()
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , firstTranspositions()
    , restElements()
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
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

void Cycle::prepareForDisjoint(word diff /*= 0*/)
{
    DisjointParamsMap disjointParams;
    fillDisjointParams(&disjointParams, diff);

    bestParams = findBestDisjointPoint(disjointParams);
}

void Cycle::fillDisjointParams(DisjointParamsMap* disjointParams, word targetDiff)
{
    uint elementCount = length();
    uint stepCount = elementCount / 2;

    disjointParams->clear();

    word minDiff = elements[0] ^ elements[1];
    uint minDiffWeight = countNonZeroBits(minDiff);

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
            uint diffWeight = uintUndefined;

            if(!targetDiff)
            {
                diffWeight = countNonZeroBits(diff);

                if(diffWeight < minDiffWeight)
                {
                    disjointParams->clear();
                    minDiffWeight = diffWeight;
                }
            }

            if(diffWeight == minDiffWeight || diff == targetDiff)
            {
                DisjointParams params = {index, step, diff, 0, 0};
                getAdditionalSumForDisjointPoint(index, step, &params);

                const uint numBufferSizeStep = 64;

                vector<DisjointParams>& paramsByDiff = (*disjointParams)[diff];
                bufferize(paramsByDiff, numBufferSizeStep);
                paramsByDiff.push_back(params);
            }
        }
    }
}

void Cycle::getAdditionalSumForDisjointPoint(uint index, uint step,
    DisjointParams* params)
{
    int sum = 0;

    word x = elements[index];
    word y = elements[modIndex(index + step)];
    word w = 0;

    // if this would be left product
    sum = 0;
    
    w = elements[modIndex(index + 1)];
    sum += countNonZeroBits(y ^ w);

    w = elements[modIndex(index + step + 1)];
    sum += countNonZeroBits(x ^ w);

    params->leftSum = sum;

    // if this would be right product
    uint elementCount = length();
    sum = 0;

    w = elements[modIndex(index + elementCount - 1)];
    sum += countNonZeroBits(w ^ y);

    w = elements[modIndex(index + step - 1)];
    sum += countNonZeroBits(w ^ x);

    params->rightSum = sum;
}

Cycle::DisjointParams Cycle::findBestDisjointPoint(const DisjointParamsMap& disjointParams)
{
    // first find the longest vector in map
    uint maxLength = 0;
    const vector<DisjointParams>* longestVector = 0;
    hasDisjointPoint = false;

    forcin(iter, disjointParams)
    {
        const vector<DisjointParams>& vectorCandidate = iter->second;
        uint elementCount = vectorCandidate.size();

        if(elementCount > maxLength)
        {
            maxLength = elementCount;
            longestVector = &vectorCandidate;
        }
    }

    // after that find in the longest vector disjoint params with the biggest step field
    DisjointParams result;
    
if(longestVector)
    {
        uint maxStep = 0;

        forcin(params, *longestVector)
        {
            uint step = params->step;
            if(step > maxStep)
            {
                maxStep = step;
                result = *params;
                hasDisjointPoint = true;
            }
        }
    }

    if(hasDisjointPoint)
    {
        // normalize result
        uint elementCount = elements.size();
        uint xIndex = result.index;
        uint yIndex = modIndex(xIndex + result.step);

        if(yIndex < xIndex)
        {
            result.index = yIndex;
            result.step = xIndex - yIndex;
        }
    }

    return result;
}

shared_ptr<list<Transposition>> Cycle::disjoint(bool isLeftMultiplication)
{
    bool makeDisjoint = true;
    shared_ptr<list<Transposition>> result;

    // first check saved results
    if(isLeftMultiplication)
    {
        if(leftDisjointResult.transpositions)
        {
            result = leftDisjointResult.transpositions;
            makeDisjoint = false;
        }
    }
    else
    {
        if(rightDisjointResult.transpositions)
        {
            result = rightDisjointResult.transpositions;
            makeDisjoint = false;
        }
    }

    // make disjoint if necessary
    if(makeDisjoint)
    {
        shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
        list<shared_ptr<Cycle>>* restCycles = 0;

        if(isLeftMultiplication)
        {
            result = leftDisjointResult.transpositions = transpositions;
            restCycles = &leftDisjointResult.restCycles;
        }
        else
        {
            result = rightDisjointResult.transpositions = transpositions;
            restCycles = &rightDisjointResult.restCycles;
        }

        disjoint(isLeftMultiplication, transpositions, restCycles);

        // debug
        cout << '\n' << (string)*this << " = ";
        if(isLeftMultiplication)
        {
            forcin(transp, *transpositions)
            {
                cout << (string)*transp;
            }
        }
        forcin(iter, *restCycles)
        {
            Cycle& cycle = **iter;
            cout << (string)cycle;
        }
        if(!isLeftMultiplication)
        {
            forcin(transp, *transpositions)
            {
                cout << (string)*transp;
            }
        }
    }

    return result;
}

void Cycle::disjoint(bool isLeftMultiplication,
    shared_ptr<list<Transposition>> transpositions,
    list<shared_ptr<Cycle>>* restCycles)
{
    if(!hasDisjointPoint)
    {
        shared_ptr<Cycle> cycle(new Cycle(*this));
        restCycles->push_back(cycle);
    }
    else
    {
        uint elementCount = length();
        list<shared_ptr<Cycle>> cycles;

        uint xIndex = bestParams.index;
        uint yIndex = xIndex + bestParams.step;
        transpositions->push_back(Transposition(elements[xIndex], elements[yIndex]));

        if(bestParams.step != 1)
        {
            vector<word> temp;
            temp.resize(bestParams.step);

            if(isLeftMultiplication)
            {
                // (x_{i+1}...x_{j-1} x_{j}}
                memcpy(temp.data(), elements.data() + xIndex + 1, bestParams.step * sizeof(word));
            }
            else
            {
                // (x_{i} x_{i+1}...x_{j-1}}
                memcpy(temp.data(), elements.data() + xIndex, bestParams.step * sizeof(word));
            }

            shared_ptr<Cycle> cycle(new Cycle(move(temp)));
            cycles.push_back(cycle);
        }

        // check for boundary conditions
        if(elementCount > 2 && elementCount - bestParams.step > 1 )
        {
            vector<word> temp;
            temp.resize(elementCount - bestParams.step);

            if(isLeftMultiplication)
            {
                // (x_{1}...x_{i-1} x_{i} x_{j+1}...x_{n}}
                memcpy(temp.data(), elements.data(), (xIndex + 1) * sizeof(word));
                memcpy(temp.data() + xIndex + 1, elements.data() + yIndex + 1,
                    (elementCount - yIndex - 1) * sizeof(word));
            }
            else
            {
                // (x_{1}...x_{i-1} x_{j} x_{j+1}...x_{n}}
                memcpy(temp.data(), elements.data(), xIndex * sizeof(word));
                memcpy(temp.data() + xIndex, elements.data() + yIndex,
                    (elementCount - yIndex) * sizeof(word));
            }

            shared_ptr<Cycle> cycle(new Cycle(move(temp)));
            cycles.push_back(cycle);
        }

        // recursion
        forin(iter, cycles)
        {
            Cycle& cycle = **iter;
            cycle.prepareForDisjoint(bestParams.diff);
            cycle.disjoint(isLeftMultiplication, transpositions, restCycles);
        }
    }
}

}   // namespace ReversibleLogic

