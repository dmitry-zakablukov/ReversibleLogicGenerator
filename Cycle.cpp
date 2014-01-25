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
    , previousMultiplicationType(mtNone)
{
}

Cycle::Cycle(vector<word>&& source)
    : elements( move(source) )
    , finalized(false)
    , distnancesSum(uintUndefined)
    , newDistancesSum(uintUndefined)
    , firstTranspositions()
    , restElements()
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
    prepareForDisjoint((word)0);

    forcin(iter, disjointParams)
    {
        word diff = iter->first;
        uint count = iter->second.size();

        if(frequencyMap->count(diff))
        {
            (*frequencyMap)[diff] += count;
        }
        else
        {
            (*frequencyMap)[diff] = count;
        }
    }
}

void Cycle::prepareForDisjoint(word diff)
{
    fillDisjointParams(&disjointParams, diff);
}

void Cycle::fillDisjointParams(DisjointParamsMap* disjointParams, word targetDiff)
{
    uint elementCount = length();
    uint stepCount = elementCount / 2;

    //// debug
    //stepCount = min(stepCount, (uint)2);

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

void Cycle::setDisjointDiff(word diff)
{
    bestParams = findBestDisjointPoint(disjointParams, diff);

    //// debug
    //forcin(iter, disjointParams)
    //{
    //    word dist = iter->first;
    //    auto v = iter->second;

    //    cout << "diff = " << dist << ", count = " << v.size() << '\n';
    //}

    //cout << "choosing diff = " << bestParams.diff << '\n';
}

Cycle::DisjointParams Cycle::findBestDisjointPoint(const DisjointParamsMap& disjointParams, word diff)
{
    // first find the longest vector in map
    const vector<DisjointParams>* longestVector = 0;
    hasDisjointPoint = false;

    if(disjointParams.count(diff))
    {
        const vector<DisjointParams>& candidate = disjointParams.at(diff);
        longestVector = &candidate;
    }
    else
    {
        uint maxLength = 0;
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
    }

    // after that find in the longest vector disjoint params with the biggest step field
    DisjointParams result;
    if(longestVector)
    {
        uint maxStep = 0;

        // debug
        maxStep = uintUndefined;

        forcin(params, *longestVector)
        {
            uint step = params->step;
            //if(step > maxStep)
            if(step < maxStep)
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

shared_ptr<list<Transposition>> Cycle::disjoint(bool isLeftMultiplication,
    uint* restCyclesDistanceSum)
{
    bool makeDisjoint = true;
    shared_ptr<list<Transposition>> result;

    // first check saved results
    if(isLeftMultiplication)
    {
        if(previousMultiplicationType == mtRightMultiplied)
        {
            result = 0;
            makeDisjoint = false;
        }
        else if(leftDisjointResult.transpositions)
        {
            result = leftDisjointResult.transpositions;
            *restCyclesDistanceSum = leftDisjointResult.restCyclesDistanceSum;
            makeDisjoint = false;
        }
    }
    else
    {
        if(previousMultiplicationType == mtLeftMultiplied)
        {
            result = 0;
            makeDisjoint = false;
        }
        else if(rightDisjointResult.transpositions)
        {
            result = rightDisjointResult.transpositions;
            *restCyclesDistanceSum = rightDisjointResult.restCyclesDistanceSum;
            makeDisjoint = false;
        }
    }

    // make disjoint if necessary
    if(makeDisjoint)
    {
        shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
        list<shared_ptr<Cycle>>* restCycles = 0;
        uint* distancesSum = 0;

        if(isLeftMultiplication)
        {
            result = leftDisjointResult.transpositions = transpositions;
            restCycles = &leftDisjointResult.restCycles;
            distancesSum = &leftDisjointResult.restCyclesDistanceSum;
        }
        else
        {
            result = rightDisjointResult.transpositions = transpositions;
            restCycles = &rightDisjointResult.restCycles;
            distancesSum = &rightDisjointResult.restCyclesDistanceSum;
        }

        disjoint(isLeftMultiplication, transpositions, restCycles);

        // calculate distances sum
        *distancesSum = 0;
        forcin(iter, *restCycles)
        {
            Cycle& cycle = **iter;
            *distancesSum += cycle.getDistancesSum();
        }

        *restCyclesDistanceSum = *distancesSum;

        //// debug
        //cout << '\n' << (string)*this << " = ";
        //if(isLeftMultiplication)
        //{
        //    forcin(transp, *transpositions)
        //    {
        //        cout << (string)*transp;
        //    }
        //}
        //forcin(iter, *restCycles)
        //{
        //    Cycle& cycle = **iter;
        //    cout << (string)cycle;
        //}
        //if(!isLeftMultiplication)
        //{
        //    forcin(transp, *transpositions)
        //    {
        //        cout << (string)*transp;
        //    }
        //}
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
            cycle.setDisjointDiff(bestParams.diff);

            cycle.disjoint(isLeftMultiplication, transpositions, restCycles);
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

uint Cycle::getDistancesSum() const
{
    uint sum = 0;
    uint elementCount = length();

    for(uint index = 0; index < elementCount - 1; ++index)
    {
        word x = elements[index];
        word y = elements[index + 1];
        sum += countNonZeroBits(x ^ y);
    }

    if(elementCount > 2)
    {
        word x = elements[0];
        word y = elements[elementCount - 1];
        sum += countNonZeroBits(x ^ y);
    }

    return sum;
}

}   // namespace ReversibleLogic

