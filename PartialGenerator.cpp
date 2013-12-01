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

    transpositionsToSynthesize->push_back(firstTransp);
    transpositionsToSynthesize->push_back(secondTransp);
}

bool PartialGenerator::isLeftAndRightMultiplicationDiffers() const
{
    assert(!transpositionsToSynthesize->size(),
        string("PartialGenerator: can't tell the difference for empty partial result"));

    bool isDiffer = false;
    forcin(iter, *transpositionsToSynthesize)
    {
        const Transposition& target = *iter;

        uint parentCycleIndex = transpToCycleIndexMap.at(target.getX());
        shared_ptr<Cycle> parentCycle = permutation.getCycle(parentCycleIndex);

        if(parentCycle->length() > 2)
        {
            isDiffer = true;
            break;
        }
    }

    return isDiffer;
}

Permutation PartialGenerator::getResidualPermutation(bool isLeftMultiplied)
{
    Permutation residualPermutation;
    return residualPermutation;
}

} //namespace ReversibleLogic