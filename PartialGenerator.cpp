#include "std.hpp"

namespace ReversibleLogic
{

PartialGenerator::PartialGenerator()
    : permutation()
    , n(uintUndefined)
    , distMap()
    , diffToEdgeMap()
{
}

PartialGenerator::~PartialGenerator()
{
}

//void PartialGenerator::setPermutation(Permutation* thePermutation, uint inputCount,
//    bool isLeftMultiplication)
//{
//    permutation = thePermutation;
//    n = inputCount;
//    leftMultiplicationFlag = isLeftMultiplication;
//
//    assert(n != uintUndefined, string("PartialGenerator: input count not defined"));
//}

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

//void PartialGenerator::prepareForGeneration()
//{
//    assert(!permutation.isEmpty(), string("PartialGenerator: preparation for empty permutation"));
//
//    shared_ptr<list<Transposition>> transpositions = getTranspositions();
//    if(!transpositions || !transpositions->size())
//    {
//        partialResultParams.type = tNone;
//    }
//    else
//    {
//        fillDistancesMap(transpositions);
//        computeEdges();
//
//        sortDistanceKeys();
//
//        // get partial result
//        word key = distKeys.front();
//        auto& candidates = distMap[key];
//
//        if(candidates->size() > 1)
//        {
//            processSameDiffTranspositions(candidates);
//        }
//        else
//        {
//            processCommonTranspositions();
//        }
//    }
//}

bool PartialGenerator::isLeftAndRightMultiplicationDiffers() const
{
    bool isDiffer = false;
    forin(iter, permutation)
    {
        const Cycle& cycle = **iter;
        if(cycle.length() > 2)
        {
            isDiffer = true;
            break;
        }
    }

    return isDiffer;
}

void PartialGenerator::prepareForGeneration()
{
    // prepare all cycles in permutation for disjoint
    unordered_map<word, uint> frequencyMap;
    forin(iter, permutation)
    {
        Cycle& cycle = **iter;
        cycle.prepareForDisjoint(&frequencyMap);
    }

    // sort keys by length
    auto sortFunction = [&](const word& left, const word& right) -> bool
    {
        uint  leftFreq = frequencyMap[ left];
        uint rightFreq = frequencyMap[right];

        bool isLess = (leftFreq > rightFreq);
        if(leftFreq == rightFreq)
        {
            isLess = (countNonZeroBits(left) < countNonZeroBits(right));
        }

        return isLess;
    };

    vector<uint> keys;
    keys.reserve(frequencyMap.size());

    forcin(iter, frequencyMap)
    {
        keys.push_back(iter->first);
    }

    sort(keys.begin(), keys.end(), sortFunction);

    // now find the best diff for disjoint
    PartialResultParams bestResult;
 
    uint keyCount = keys.size();
    for(uint index = 0; index < keyCount; ++index)
    {
        word diff = keys[index];
        uint freq = frequencyMap[diff];

        if(freq < bestResult.getCoveredTranspositionsCount())
        {
            // even theoretically we can't get more transpositions than we already have on previous step
            break;
        }

        shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
        forin(iter, permutation)
        {
            Cycle& cycle = **iter;
            cycle.disjointByDiff(diff, transpositions);
        }

        uint transpositionCount = transpositions->size();
        if(transpositionCount < bestResult.getCoveredTranspositionsCount()
            || transpositionCount == 1)
        {
            // real number of transpositions is lower than theoretical, so go to next diff
            // also skip this step if transposition count equals 1
            continue;
        }

        PartialResultParams result = getPartialResult(transpositions, diff, bestResult);
        if(result.isBetterThan(bestResult))
        {
            bestResult = result;
        }
    }

    if(bestResult.type == PartialResultParams::tNone)
    {
        shared_ptr<list<Transposition>> transpositions(new list<Transposition>);

        uint keyCount = keys.size();
        for(uint index = 0; index < keyCount && transpositions->size() < 2; ++index)
        {
            word diff = keys[index];
            forin(iter, permutation)
            {
                Cycle& cycle = **iter;
                cycle.disjointByDiff(diff, transpositions);
            }
        }
        // bugbug: see processCommonTranspositions() for better pair creation

        assert(transpositions->size() == 2,
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
}

PartialResultParams PartialGenerator::getPartialResult(
    shared_ptr<list<Transposition>> transpositions, word diff,
    const PartialResultParams& bestParams)
{
    BooleanEdgeSearcher edgeSearcher(transpositions, n, diff);
    BooleanEdge edge = edgeSearcher.findEdge();

    PartialResultParams result;
    if(edge.isValid())
    {
        word capacity = edge.getCapacity();
        if(capacity >= bestParams.params.edgeCapacity)
        {
            result.type = (edge.isFull() ? PartialResultParams::tFullEdge : PartialResultParams::tEdge);
            result.transpositions = edgeSearcher.getEdgeSubset(edge, n, transpositions);
            result.edge = edge;

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

shared_ptr<list<Transposition>> PartialGenerator::getTranspositions()
{
    shared_ptr<list<Transposition>> transpositions(new list<Transposition>);
    partialResultParams.restCyclesDistanceSum = 0;

    forin(iter, permutation)
    {
        Cycle& cycle = **iter;
        uint cycleDistancesSum = 0;

        shared_ptr<list<Transposition>> cycleTranspositions = 0;
            //cycle.disjoint(leftMultiplicationFlag, &cycleDistancesSum);

        if(cycleTranspositions)
        {
            transpositions->insert(transpositions->end(),
                cycleTranspositions->cbegin(), cycleTranspositions->cend());

            partialResultParams.restCyclesDistanceSum += cycleDistancesSum;
        }
    }

    if(transpositions && transpositions->size() == 1)
    {
        assert(permutation.length() == 1,
            string("PartialGenerator::getTranspositions() failed because my DNA"));

        shared_ptr<Cycle> cycle = permutation.getCycle(0);
        cycle->completeDisjoint(leftMultiplicationFlag, transpositions);
    }

    return transpositions;
}

void PartialGenerator::fillDistancesMap(shared_ptr<list<Transposition>> transpositions)
{
    decltype(distMap) emptyMap;
    distMap = emptyMap;

    forcin(iter, *transpositions)
    {
        addTranspToDistMap(*iter);
    }
}

word PartialGenerator::addTranspToDistMap( const Transposition& transp )
{
    word diff = transp.getDiff();
    if(!distMap.count(diff))
    {
        shared_ptr<list<Transposition>> newList(new list<Transposition>);
        distMap[diff] = newList;
    }

    bool alreadyHasTransp = false;
    auto transpositions = distMap[diff];

    forin(iter, *transpositions)
    {
        const Transposition& t = *iter;
        alreadyHasTransp = alreadyHasTransp || (t == transp);
    }

    if(!alreadyHasTransp)
    {
        distMap[diff]->push_back(transp);
        diffToEdgeMap.erase(diff);
    }

    return diff;
}

void PartialGenerator::computeEdges()
{
    forcin(iter, distMap)
    {
        word diff = iter->first;
        shared_ptr<list<Transposition>> transpositions = iter->second;

        if(!diffToEdgeMap.count(diff))
        {
            BooleanEdgeSearcher edgeSearcher(transpositions, n, diff);
            BooleanEdge edge = edgeSearcher.findEdge();

            diffToEdgeMap[diff] = edge;
        }
    }
}

void PartialGenerator::sortDistanceKeys()
{
    struct DiffSortKey
    {
        word diff;
        uint length;
        uint weight;
        word capacity;
        //bool isGood = false;
    };

    vector<DiffSortKey> keys(distMap.size());
    uint index = 0;

    forcin(iter, distMap)
    {
        DiffSortKey key;

        key.diff = iter->first;
        key.weight = countNonZeroBits(key.diff);
        key.length = (iter->second)->size();

        //key.isGood = (key.length > 1);

        BooleanEdge edge = computeEdge(key.diff);
        key.capacity = edge.getCapacity();

        keys[index] = key;
        ++index;
    }

    auto sortFunc = [](const DiffSortKey& leftKey, const DiffSortKey& rightKey) -> bool
    {        
        ////bool isLess = leftKey.length > rightKey.length;
        //////bool isLess = (leftKey.length > 1 ) && (rightKey.length == 1);
        ////if(leftKey.length == rightKey.length)
        ////{
        ////    isLess = leftKey.weight < rightKey.weight;
        ////}        

        bool isLess = (leftKey.capacity > rightKey.capacity);
        if(leftKey.capacity == rightKey.capacity)
        {
            ///isLess = leftKey.weight < rightKey.weight;
            isLess = (leftKey.length > 1 ) && (rightKey.length == 1);
            if(leftKey.length == rightKey.length)
            {
                isLess = leftKey.weight < rightKey.weight;
            }     
        }

        return isLess;
    };

    sort(keys.begin(), keys.end(), sortFunc);

    distKeys = list<word>(); 
    forin(iter, keys)
    {
        const DiffSortKey& key = *iter;
        distKeys.push_back(key.diff);
    }
}

BooleanEdge PartialGenerator::computeEdge(word diff, bool force /*= false*/)
{
    BooleanEdge edge;
    assert(distMap.count(diff), string("Distance map hasn't difference"));

    if(!diffToEdgeMap.count(diff) || force)
    {
        auto transpositions = distMap[diff];

        BooleanEdgeSearcher edgeSearcher(transpositions, n, diff);
        edge = edgeSearcher.findEdge();

        diffToEdgeMap[diff] = edge;
    }
    else
    {
        edge = diffToEdgeMap[diff];
    }

    return edge;
}

void PartialGenerator::processSameDiffTranspositions(shared_ptr<list<Transposition>> candidates)
{
    word initialMask = 0;
    assert(candidates && candidates->size(), string("PartialGenerator: empty candidates"));

    const Transposition& transp = candidates->front();
    initialMask = transp.getDiff();

    BooleanEdge edge;
    if(diffToEdgeMap.count(initialMask))
    {
        edge = diffToEdgeMap[initialMask];
    }
    else
    {
        BooleanEdgeSearcher edgeSearcher(candidates, n, initialMask);
        edge = edgeSearcher.findEdge();
    }

    if(edge.isValid() && edge.getCapacity() > 2)
    {
        partialResultParams.params.edgeCapacity = edge.getCapacity();
        
        if(edge.isFull())
        {
            partialResultParams.type = PartialResultParams::tFullEdge;
            transpositionsToSynthesize = candidates;
        }
        else
        {
            partialResultParams.type = PartialResultParams::tEdge;
            transpositionsToSynthesize = BooleanEdgeSearcher::getEdgeSubset(edge, n, candidates);
        }
    }
    else
    {
        partialResultParams.type = PartialResultParams::tSameDiffPair;
        partialResultParams.params.diff = initialMask;

        transpositionsToSynthesize = findBestCandidates(candidates);
    }
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

    forin(iter, *candidates)
    {
        const Transposition& transp = *iter;

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
    forin(iter, *candidates)
    {
        *iter = keys[index].transp;
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
    forin(iter, *candidates)
    {
        const Transposition& cand = *iter;

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

void PartialGenerator::processCommonTranspositions()
{
    // 1) find first
    word diff = distKeys.front();
    Transposition firstTransp = distMap[diff]->front();

    // 2) find second
    uint minComplexity = uintUndefined;
    Transposition secondTransp;

    forin(iter, distKeys)
    {
        const word& key = *iter;
        if(key == diff)
        {
            continue;
        }

        // no skipping keys here because we don't know if partial result
        // would be left or right multiplied

        Transposition transp = distMap[key]->front();
        TransposPair pair = TransposPair(firstTransp, transp);
        pair.setN(n);

        uint complexity = pair.getEstimateImplComplexity();
        if(minComplexity == uintUndefined || complexity < minComplexity)
        {
            minComplexity = complexity;
            secondTransp = transp;
        }
    }

    assert(!secondTransp.isEmpty(), string("Second transposition is empty"));

    // fill partial result parameters
    partialResultParams.type = PartialResultParams::tCommonPair;

    word leftDiff  = firstTransp.getDiff();
    word rightDiff = secondTransp.getDiff();

    word leftX  = firstTransp.getX();
    word rightX = secondTransp.getX();
    word distance = (leftX & (~leftDiff)) ^ (rightX & (~rightDiff));

    partialResultParams.params.common.leftDiff  = leftDiff;
    partialResultParams.params.common.rightDiff = rightDiff;
    partialResultParams.params.common.distance  = distance;

    transpositionsToSynthesize = shared_ptr<list<Transposition>>(new list<Transposition>);
    transpositionsToSynthesize->push_back(firstTransp);
    transpositionsToSynthesize->push_back(secondTransp);
}

Permutation PartialGenerator::getResidualPermutation() const
{
    return getResidualPermutation(leftMultiplicationFlag);
}

ReversibleLogic::Permutation PartialGenerator::getResidualPermutation(bool isLeftMultiplication) const
{
    vector<shared_ptr<Cycle>> cycles;
    forin(iter, permutation)
    {
        const Cycle& cycle = **iter;

        cycle.multiplyByTranspositions(partialResultParams.transpositions,
            isLeftMultiplication, &cycles);
    }

    Permutation residualPermutation(cycles);
    return residualPermutation;
}

deque<ReverseElement> PartialGenerator::implementPartialResult()
{
    transpositionsToSynthesize = partialResultParams.transpositions;
    assert(transpositionsToSynthesize->size(), string("PartialGenerator: no transpositions to synthesize"));

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
    const Transposition& transp = transpositionsToSynthesize->front();
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
    assert(transpositionsToSynthesize->size() == 2,
        string("PartialGenerator: can't implement pair of transpositions"));

    Transposition firstTransp  = transpositionsToSynthesize->front();
    Transposition secondTransp = transpositionsToSynthesize->back();

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