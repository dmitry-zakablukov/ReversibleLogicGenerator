#include "std.hpp"

namespace ReversibleLogic
{

PartialGenerator::PartialGenerator()
    : permutation()
    , n(uintUndefined)
{
}

PartialGenerator::~PartialGenerator()
{
}

void PartialGenerator::setPermutation(const Permutation& thePermutation, uint inputCount)
{
    permutation = thePermutation;
    n = inputCount;

    assert(n != uintUndefined, string("PartialGenerator: input count not defined"));
}

const Permutation& PartialGenerator::getPermutation() const
{
    return permutation;
}

bool PartialGenerator::isLeftAndRightMultiplicationDiffers() const
{
    bool isDiffer = false;
    for (auto cycle : permutation)
    {
        if(cycle->length() > 2)
        {
            isDiffer = true;
            break;
        }
    }

    if (isDiffer)
    {
        uint length = 0;
        for (auto cycle : permutation)
            length += cycle->length();

        isDiffer = (length > 3);
    }

    return isDiffer;
}

void PartialGenerator::prepareForGeneration()
{
    // prepare all cycles in permutation for disjoint
    unordered_map<word, uint> frequencyMap;
    for (auto cycle : permutation)
    {
        cycle->prepareForDisjoint(&frequencyMap);
    }

    // sort keys by length
    auto sortFunction = [&](const word& left, const word& right) -> bool
    {
        uint  leftWeight = countNonZeroBits( left);
        uint rightWeight = countNonZeroBits(right);

        uint  leftFreq = frequencyMap[ left];
        uint rightFreq = frequencyMap[right];

        bool isLess = (leftFreq > rightFreq);
        if(leftFreq == rightFreq)
        {
            isLess = (leftWeight < rightWeight);
        }

        //bool isLess = (leftWeight < rightWeight);
        //if(leftWeight == rightWeight)
        //{
        //    isLess = (leftFreq > rightFreq);
        //}

        return isLess;
    };

    vector<uint> keys;
    keys.reserve(frequencyMap.size());

    for (auto iter : frequencyMap)
    {
        keys.push_back(iter.first);
    }

    sort(keys.begin(), keys.end(), sortFunction);

    // now find the best diff for disjoint
    PartialResultParams bestResult;
    uint weight = uintUndefined;
 
    uint keyCount = keys.size();
    for(uint index = 0; index < keyCount; ++index)
    {
        word diff = keys[index];
        uint freq = frequencyMap[diff];

        //if(weight == uintUndefined)
        //{
        //    weight = countNonZeroBits(diff);
        //}
        //else if(countNonZeroBits(diff) > weight)
        //{
        //    break;
        //}

        if(freq < bestResult.getCoveredTranspositionsCount())
        {
            // even theoretically we can't get more transpositions than we already have on previous step
            //break;
        }

        shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
        for (auto cycle : permutation)
        {
            cycle->disjointByDiff(diff, transpositions);
        }

        uint transpositionCount = transpositions->size();
        //if(transpositionCount < bestResult.getCoveredTranspositionsCount()
        //    || transpositionCount == 1)
        if(transpositionCount == 1)
        {
            // real number of transpositions is lower than theoretical, so go to next diff
            // also skip this step if transposition count equals 1
            continue;
        }

        PartialResultParams result = getPartialResult(transpositions, diff, bestResult);
        if(result.edge.coveredTranspositionCount > bestResult.edge.coveredTranspositionCount)
        {
            bestResult = result;
        }
    }

    if(bestResult.type == PartialResultParams::tNone)
    {
        // bugbug: see processCommonTranspositions() for better pair creation
        shared_ptr<list<Transposition>> transpositions = getCommonPair();

        assert(transpositions->size() > 1,
            string("PartialGenerator::prepareForGeneration() failed to find common pair"));

        bestResult.type = PartialResultParams::tCommonPair;
        bestResult.transpositions = transpositions;

        Transposition& firstTransp = transpositions->front();
        Transposition& secondTransp = transpositions->back();

        word leftDiff  =  firstTransp.getDiff();
        word rightDiff = secondTransp.getDiff();

        word leftX  = firstTransp.getX();
        word rightX = secondTransp.getX();
        word distance = (leftX & (~leftDiff)) ^ (rightX & (~rightDiff));

        bestResult.params.common.leftDiff  = leftDiff;
        bestResult.params.common.rightDiff = rightDiff;
        bestResult.params.common.distance  = distance;
    }

    partialResultParams = bestResult;
    partialResultParams.distancesSum = permutation.getDistancesSum();
}

shared_ptr<list<Transposition>> PartialGenerator::getCommonPair()
{
    shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
    if (permutation.length() > 1)
    {
        auto iter = permutation.begin();
        Cycle& firstCycle = **iter;
        iter++;
        Cycle& secondCycle = **iter;

        transpositions->push_back(Transposition(firstCycle[0], firstCycle[1]));
        transpositions->push_back(Transposition(secondCycle[0], secondCycle[1]));
    }
    else
    {
        Cycle& cycle = **(permutation.begin());
        if (cycle.length() >= 4)
        {
            transpositions->push_back(Transposition(cycle[0], cycle[1]));
            transpositions->push_back(Transposition(cycle[2], cycle[3]));
        }
        else
        {
            transpositions->push_back(Transposition(cycle[0], cycle[1]));
            transpositions->push_back(Transposition(cycle[0], cycle[2]));
        }
    }

    return transpositions;
}

PartialResultParams PartialGenerator::getPartialResult(
    shared_ptr<list<Transposition>> transpositions, word diff,
    const PartialResultParams& bestParams)
{
    BooleanEdgeSearcher edgeSearcher(transpositions, n, diff);
    BooleanEdge edge = edgeSearcher.findEdge();

    PartialResultParams result;
    if(edge.isValid() && edge.getCapacity() > 2)
    {
        word capacity = edge.getCapacity();
        if(capacity >= bestParams.params.edgeCapacity)
        {
            result.type = (edge.isFull() ? PartialResultParams::tFullEdge : PartialResultParams::tEdge);
            result.transpositions = edgeSearcher.getEdgeSubset(edge, n);

            edge = BooleanEdgeSearcher(result.transpositions, n, diff).findEdge();
            result.edge = edge;
            //result.edge = BooleanEdgeSearcher(result.transpositions, n, diff).findEdge();

            result.params.diff = diff;
            result.params.edgeCapacity = edge.getCapacity();
        }
    }
    else if(!bestParams.params.edgeCapacity)
    {
        result.type = PartialResultParams::tSameDiffPair;
        result.transpositions = findBestCandidates(transpositions);

        result.params.diff = diff;
    }

    return result;
}


PartialResultParams PartialGenerator::getPartialResultParams() const
{
    return partialResultParams;
}

shared_ptr<list<Transposition>> PartialGenerator::findBestCandidates(shared_ptr<list<Transposition>> candidates)
{
    sortCandidates(candidates);

    uint candidateCount = candidates->size();
    assert(candidateCount > 1, string("PartialGenerator: too few candidates for findBestCandidates()"));

    auto iter = candidates->begin();

    // find best partner for first transposition among candidates
    Transposition firstPartner;
    uint firstDist;

    tie(firstPartner, firstDist) = findBestCandidatePartner(candidates, *iter);
    ++iter;

    // find best partner for second transposition among candidates
    Transposition secondPartner;
    uint secondDist;

    tie(secondPartner, secondDist) = findBestCandidatePartner(candidates, *iter);

    Transposition first;
    Transposition second;

    if(firstDist <= secondDist)
    {
        first  = candidates->front();
        second = firstPartner;
    }
    else
    {
        auto iter = candidates->begin();
        ++iter;

        first  = *iter;
        second = secondPartner;
    }

    shared_ptr<list<Transposition>> result(new list<Transposition>);
    result->push_back(first);
    result->push_back(second);

    return result;
}

void PartialGenerator::sortCandidates( shared_ptr<list<Transposition>> candidates )
{
    struct CandSortKey
    {
        uint weight;
        Transposition transp;
    };

    vector<CandSortKey> keys(candidates->size());
    uint index = 0;

    for (auto& transp : *candidates)
    {
        word minValue = min(transp.getX(), transp.getY());
        keys[index].weight = countNonZeroBits(minValue);
        keys[index].transp = transp;
        ++index;
    }

    auto sortFunc = [](const CandSortKey& leftKey, const CandSortKey& rightKey)
    {
        return leftKey.weight < rightKey.weight;
    };

    sort(keys.begin(), keys.end(), sortFunc);

    index = 0;
    for (auto& t : *candidates)
    {
        t = keys[index].transp;
        ++index;
    }
}

tuple<Transposition, uint>
PartialGenerator::findBestCandidatePartner(
    const shared_ptr<list<Transposition>> candidates, const Transposition& target)
{
    Transposition second;
    uint minDist = uintUndefined;

    auto count = countNonZeroBits;
    for (auto& cand : *candidates)
    {
        uint dxz = uintUndefined;
        uint dxw = uintUndefined;
        uint dyz = uintUndefined;
        uint dyw = uintUndefined;

        if(cand != target)
        {
            dxz = count(target.getX() ^ cand.getX());
            dxw = count(target.getX() ^ cand.getY());
            dyz = count(target.getY() ^ cand.getX());
            dyw = count(target.getY() ^ cand.getY());

            uint dist = min(dxz, dxw);
            dist = min(dist, dyz);
            dist = min(dist, dyw);

            if(minDist == uintUndefined || dist < minDist)
            {
                minDist = dist;
                second = cand;
            }
        }
    }

    assert(!second.isEmpty(), string("Candidate partner not found"));
    return tie(second, minDist);
}

//void PartialGenerator::processCommonTranspositions()
//{
//    // 1) find first
//    word diff = distKeys.front();
//    Transposition firstTransp = distMap[diff]->front();
//
//    // 2) find second
//    uint minComplexity = uintUndefined;
//    Transposition secondTransp;
//
//    forin(iter, distKeys)
//    {
//        const word& key = *iter;
//        if(key == diff)
//        {
//            continue;
//        }
//
//        // no skipping keys here because we don't know if partial result
//        // would be left or right multiplied
//
//        Transposition transp = distMap[key]->front();
//        TransposPair pair = TransposPair(firstTransp, transp);
//        pair.setN(n);
//
//        uint complexity = pair.getEstimateImplComplexity();
//        if(minComplexity == uintUndefined || complexity < minComplexity)
//        {
//            minComplexity = complexity;
//            secondTransp = transp;
//        }
//    }
//
//    assert(!secondTransp.isEmpty(), string("Second transposition is empty"));
//
//    // fill partial result parameters
//    partialResultParams.type = PartialResultParams::tCommonPair;
//
//    word leftDiff  = firstTransp.getDiff();
//    word rightDiff = secondTransp.getDiff();
//
//    word leftX  = firstTransp.getX();
//    word rightX = secondTransp.getX();
//    word distance = (leftX & (~leftDiff)) ^ (rightX & (~rightDiff));
//
//    partialResultParams.params.common.leftDiff  = leftDiff;
//    partialResultParams.params.common.rightDiff = rightDiff;
//    partialResultParams.params.common.distance  = distance;
//
//    transpositionsToSynthesize = shared_ptr<list<Transposition>>(new list<Transposition>);
//    transpositionsToSynthesize->push_back(firstTransp);
//    transpositionsToSynthesize->push_back(secondTransp);
//}

ReversibleLogic::Permutation PartialGenerator::getResidualPermutation(bool isLeftMultiplication) const
{
    //vector<shared_ptr<Cycle>> cycles;
    //forin(iter, permutation)
    //{
    //    const Cycle& cycle = **iter;

    //    cycle.multiplyByTranspositions(partialResultParams.transpositions,
    //        isLeftMultiplication, &cycles);
    //}

    //Permutation residualPermutation(cycles);
    //return residualPermutation;

    return permutation.multiplyByTranspositions(partialResultParams.transpositions, isLeftMultiplication);
}

deque<ReverseElement> PartialGenerator::implementPartialResult()
{
    assert(partialResultParams.transpositions->size(), string("PartialGenerator: no transpositions to synthesize"));

    deque<ReverseElement> synthesisResult;
    switch(partialResultParams.type)
    {
    case PartialResultParams::tFullEdge:
    case PartialResultParams::tEdge:
        synthesisResult = implementEdge();
        break;

    case PartialResultParams::tSameDiffPair:
    case PartialResultParams::tCommonPair:
        synthesisResult = implementPairOfTranspositions();
        break;
    }   

    return synthesisResult;
}

deque<ReverseElement> PartialGenerator::implementEdge()
{
    const Transposition& transp = partialResultParams.transpositions->front();
    word diff = transp.getDiff();

    BooleanEdge& edge = partialResultParams.edge;

    word baseValue = edge.getBaseValue(n);
    word baseMask  = edge.getBaseMask(n);

    deque<ReverseElement> elements;
    word mask = 1;

    while(mask <= diff)
    {
        if(diff & mask)
        {
            ReverseElement element(n, mask, baseMask, (~baseValue) & baseMask);
            elements.push_back(element);
        }

        mask <<= 1;
    }

    return elements;
}

deque<ReverseElement> PartialGenerator::implementPairOfTranspositions()
{
    assert(partialResultParams.transpositions->size() == 2,
        string("PartialGenerator: can't implement pair of transpositions"));

    Transposition firstTransp  = partialResultParams.transpositions->front();
    Transposition secondTransp = partialResultParams.transpositions->back();

    deque<ReverseElement> elements;

    auto firstRealization = implementSingleTransposition(firstTransp);
    elements.insert(elements.end(), firstRealization.cbegin(), firstRealization.cend());

    auto secondRealization = implementSingleTransposition(secondTransp);
    elements.insert(elements.end(), secondRealization.cbegin(), secondRealization.cend());

    return elements;
}

deque<ReverseElement> PartialGenerator::implementSingleTransposition(const Transposition& transp)
{
    /// New method: use maximum control inputs as possible with inversion
    deque<ReverseElement> conjugationElements;
    deque<ReverseElement> elements;

    word x = transp.getX();
    word y = transp.getY();
    word fullMask = ((word)1 << n) - 1;

    word diff = x ^ y;
    uint pos = findPositiveBitPosition(diff);

    word targetMask = (word)1 << pos;
    diff ^= targetMask;

    // main element
    word mainElementInversionMask = 0;
    if(x & targetMask) // target in B_10 (x_i == 1, y_i == 0)
    {
        mainElementInversionMask =
            (~(x ^ diff)) & fullMask;
    }
    else // target in B_01 (x_i == 0, y_i == 1)
    {
        mainElementInversionMask =
            (~(y ^ diff)) & fullMask;
    }

    ReverseElement element(n, targetMask, fullMask ^ targetMask,
        mainElementInversionMask);

    elements.push_back(element);

    if(diff)
    {
        word mask = targetMask << 1;
        while(mask <= diff)
        {
            if(diff & mask)
            {
                ReverseElement element(n, mask, targetMask);
                conjugationElements.push_back(element);
            }
            mask <<= 1;
        }

        elements = conjugate(elements, conjugationElements, true);
    }

    return elements;
}

} //namespace ReversibleLogic