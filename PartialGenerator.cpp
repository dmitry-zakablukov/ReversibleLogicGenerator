#include "std.hpp"

namespace ReversibleLogic
{

PartialGenerator::PartialGenerator()
    : permutation()
    , n(uintUndefined)
    , distMap()
    , transpToCycleIndexMap()
    , diffToEdgeMap()
{

}

PartialGenerator::~PartialGenerator()
{

}

void PartialGenerator::setPermutation(Permutation thePermutation, uint inputCount)
{
    permutation = thePermutation;
    n = inputCount;

    assert(n != uintUndefined, string("PartialGenerator: input count not defined"));
}

void PartialGenerator::prepareForGeneration()
{
    assert(!permutation.isEmpty(), string("PartialGenerator: preparation for empty permutation"));

    fillDistancesMap();
    computeEdges();

    sortDistanceKeys();

    // get partial result
    word key = distKeys.front();
    auto& candidates = distMap[key];

    if(candidates->size() > 1)
    {
        processSameDiffTranspositions(candidates);
    }
    else
    {
        processCommonTranspositions();
    }
}

PartialResultParams PartialGenerator::getPartialResultParams() const
{
    return partialResultParams;
}

void PartialGenerator::fillDistancesMap()
{
    decltype(distMap) emptyMap;
    distMap = emptyMap;

    word vectorSize = 1 << n;

    uint cycleCount = permutation.length();
    for(uint cycleIndex = 0; cycleIndex < cycleCount; ++cycleIndex)
    {
        auto cycle = permutation.getCycle(cycleIndex);
        shared_ptr<list<Transposition>> transpositions = cycle->getBestTranspositionsForDisjoint();

        forcin(iter, *transpositions)
        {
            addTranspToDistMap(*iter);
        }

        uint elementCount = cycle->length();
        for(uint elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const word& element = (*cycle)[elementIndex];
            transpToCycleIndexMap[element] = cycleIndex;
        }
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
            partialResultParams.type = tFullEdge;
            transpositionsToSynthesize = candidates;
        }
        else
        {
            partialResultParams.type = tEdge;
            transpositionsToSynthesize = BooleanEdgeSearcher::getEdgeSubset(edge, n, candidates);
        }
    }
    else
    {
        partialResultParams.type = tSameDiffPair;
        partialResultParams.params.diff = initialMask;

        findBestCandidates(candidates);
    }
}

void PartialGenerator::findBestCandidates(shared_ptr<list<Transposition>> candidates)
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

    transpositionsToSynthesize = shared_ptr<list<Transposition>>(new list<Transposition>);
    transpositionsToSynthesize->push_back(first);
    transpositionsToSynthesize->push_back(second);
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
    partialResultParams.type = tCommonPair;

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

bool PartialGenerator::isLeftAndRightMultiplicationDiffers() const
{
    assert(transpositionsToSynthesize->size(),
        string("PartialGenerator: can't tell the difference for empty partial result"));

    bool isDiffer = false;
    forcin(target, *transpositionsToSynthesize)
    {
        uint parentCycleIndex = transpToCycleIndexMap.at(target->getX());
        shared_ptr<Cycle> parentCycle = permutation.getCycle(parentCycleIndex);

        if(parentCycle->length() > 2)
        {
            isDiffer = true;
            break;
        }
    }

    return isDiffer;
}

Permutation PartialGenerator::getResidualPermutation(bool isLeftMultiplication)
{
    // 1) remember all elements in transpositions to remove
    unordered_set<word> targetElements;
    forcin(transp, *transpositionsToSynthesize)
    {
        targetElements.insert(transp->getX());
        targetElements.insert(transp->getY());
    }

    // 2) remember all cycle indices which should be reduced
    // and difference for this cycle
    unordered_set<uint> cycleIndicesToReduce;
    unordered_map<uint, word> cycleIndexToDiff;

    forcin(transp, *transpositionsToSynthesize)
    {
        uint cycleIndex = transpToCycleIndexMap[transp->getX()];
        if(!cycleIndicesToReduce.count(cycleIndex))
        {
            cycleIndicesToReduce.insert(cycleIndex);
            cycleIndexToDiff[cycleIndex] = transp->getDiff();
        }
    }

    // 3) loop on cycles and reduce them if needed
    uint cycleCount = permutation.length();

    vector<shared_ptr<Cycle>> newCycles;
    newCycles.reserve(cycleCount);

    for(uint index = 0; index < cycleCount; ++index)
    {
        shared_ptr<Cycle> nextCycle = permutation.getCycle(index);

        if(cycleIndicesToReduce.count(index))
        {
            word diff = cycleIndexToDiff[index];

            shared_ptr<Cycle> reducedCycle =
                nextCycle->multiplyByTranspositions(targetElements, diff, isLeftMultiplication);

            if(reducedCycle && reducedCycle->length())
            {
                newCycles.push_back(reducedCycle);
            }
        }
        else if(nextCycle->length())
        {
            newCycles.push_back(nextCycle);
        }
    }

    // 4) create residual permutation based on new cycles
    Permutation residualPermutation(newCycles);
    return residualPermutation;
}

deque<ReverseElement> PartialGenerator::implementPartialResult()
{
    assert(transpositionsToSynthesize->size(), string("PartialGenerator: no transpositions to synthesize"));

    deque<ReverseElement> synthesisResult;
    switch(partialResultParams.type)
    {
    case tFullEdge:
    case tEdge:
        synthesisResult = implementEdge();
        break;

    case tSameDiffPair:
    case tCommonPair:
        synthesisResult = implementPairOfTranspositions();
        break;
    }
   

    return synthesisResult;
}

deque<ReverseElement> PartialGenerator::implementEdge()
{
    const Transposition& transp = transpositionsToSynthesize->front();
    word diff = transp.getDiff();

    BooleanEdge edge = diffToEdgeMap[diff];

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