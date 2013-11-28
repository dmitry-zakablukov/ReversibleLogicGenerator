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
        //diffToEdgeMap.erase(initialMask);
    }
    else
    {
        BooleanEdgeSearcher edgeSearcher(candidates, n, initialMask);
        edge = edgeSearcher.findEdge();
    }

    if(edge.isValid() && edge.getCapacity() > 2)
    {
        partialResult.params.edgeCapacity = edge.getCapacity();
        
        if(edge.isFull())
        {
            partialResult.type = tFullEdge;
            resultTranspositions = candidates;
        }
        else
        {
            partialResult.type = tEdge;
            resultTranspositions = BooleanEdgeSearcher::getEdgeSubset(edge, n, candidates);
        }
    }
    else
    {
        partialResult.type = tSameDiffPair;
        partialResult.params.diff = initialMask;

        findBestCandidates(candidates);
    }
}

void PartialGenerator::findBestCandidates(shared_ptr<list<Transposition>> candidates)
{
    sortCandidates(candidates);

    // TODO:
    auto iter = candidates->begin();
    resultTranspositions->push_back(*iter++);
    resultTranspositions->push_back(*iter);
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

        // TODO:
        //if(skipKeyFound && key == skipKeyValue)
        //{
        //    continue;
        //}

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

    partialResult.type = tCommonPair;
    // TODO:
    partialResult.params.common.leftDiff  =  firstTransp.getDiff();
    partialResult.params.common.rightDiff = secondTransp.getDiff();
    partialResult.params.common.distance  = (firstTransp.getX() & (~firstTransp.getDiff()))
        ^ (secondTransp.getX() & (~secondTransp.getDiff()));

    resultTranspositions->push_back(firstTransp);
    resultTranspositions->push_back(secondTransp);
}

} //namespace ReversibleLogic