#include "std.hpp"

namespace ReversibleLogic
{

Generator::Generator()
    : n(0)
    , permutation()
    , distMap()
    , distKeys()
    , candidatesSorted(false)
    , transpToCycleIndexMap()
    , diffToEdgeMap()
    , log(0)
{
}

Generator::Scheme Generator::generate(const PermutationTable& table, ostream& outputLog)
{
    log = &outputLog;

    float totalTime = 0;
    float time = 0;

    n = 0;
    {
        AutoTimer timer(&time);

        checkPermutationValidity(table);

        tie(n, permutation) = getPermutation(table);
    }

    *log << "Permutation creation time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    //debug
    *log << (string)permutation << "\n";

    Scheme scheme;
    candidatesSorted = false;

    //debug
    //return scheme;

    time = 0;
    {
        AutoTimer timer( &time );

        fillDistancesMap();
        computeEdges();

        sortDistanceKeys();

        while(!permutation.isEmpty())
        {
            ////debug
            //*log << (string)permutation << "\n";

            word key = distKeys.front();
            auto& candidates = distMap[key];

            if(candidates->size() > 1)
            {
                auto elements = implementCandidates(candidates);
                scheme.insert(scheme.end(), elements.cbegin(), elements.cend());
            }
            else
            {
                auto elements = implementCommonPair();
                scheme.insert(scheme.end(), elements.cbegin(), elements.cend());
            }
        }
    }

    //////debug: test generation
    ////scheme.resize(0);
    ////TransposPair pair( Transposition(5, 13, false), Transposition(7, 15, false) );
    ////pair.setN(n);
    ////auto elements = pair.getImplementation();
    ////scheme.insert(scheme.end(), elements.cbegin(), elements.cend());

    ////pair = TransposPair( Transposition(5, 7, false), Transposition(13, 15, false) );
    ////pair.setN(n);
    ////elements = pair.getImplementation();
    ////scheme.insert(scheme.end(), elements.cbegin(), elements.cend());

    ////pair = TransposPair( Transposition(5, 13, false), Transposition(9, 11, false) );
    ////pair.setN(n);
    ////elements = pair.getImplementation();
    ////scheme.insert(scheme.end(), elements.cbegin(), elements.cend());

    *log << "Scheme synthesis time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    time = 0;
    {
        AutoTimer timer(&time);

        *log << "Complexity before optimization: " << scheme.size() << '\n';

        PostProcessor optimizer;

        uint elementCount = scheme.size();
        vector<ReverseElement> optimizedScheme(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            optimizedScheme[index] = scheme[index];
        }

        optimizedScheme = optimizer.optimize(optimizedScheme);

        elementCount = optimizedScheme.size();
        scheme.resize(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            scheme[index] = optimizedScheme[index];
        }

        *log << "Complexity after optimization: " << scheme.size() << '\n';

        bool isValid = checkSchemeAgainstPermutationVector(scheme, table);
        //debug
        assert(isValid, string("Generated scheme is not valid"));
    }

    *log << "Optimization time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    *log << "Total time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << totalTime / 1000;
    *log << " sec\n";

    //string repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    //repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    return scheme;
}

void Generator::checkPermutationValidity(const PermutationTable& table)
{
    set<word> inputs;
    set<word> outputs;

    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        const word& y = table[x];

        inputs.insert(x);
        outputs.insert(y);
    }

    assert(inputs.size() == table.size(),
           string("Number of inputs in permutation table is too small"));

    assert(outputs.size() == table.size(),
           string("Number of outputs in permutation table is too small"));
}

tuple<uint, Permutation> Generator::getPermutation(const PermutationTable& table)
{
    Permutation permutation = PermutationUtils::createPermutation(table);

    word maxValue = 0;
    forin(cycleIter, permutation)
    {
        const shared_ptr<Cycle>& cycle = *cycleIter;
        uint elementCount = cycle->length();
        for(uint index = 0; index < elementCount; ++index)
        {
            const word& element = (*cycle)[index];
            maxValue |= element;
        }
    }

    // find number of bits
    uint n = 0;
    word mask = 1;

    while(mask <= maxValue)
    {
        ++n;
        mask <<= 1;
    }

    return tie(n, permutation);
}

void Generator::fillDistancesMap()
{
    distMap = map<word, shared_ptr<list<Transposition>>>();

    word vectorSize = 1 << n;
    transpToCycleIndexMap.resize(vectorSize);

    uint cycleCount = permutation.length();
    for(uint cycleIndex = 0; cycleIndex < cycleCount; ++cycleIndex)
    {
        auto cycle = permutation.getCycle(cycleIndex);
        Transposition transp = cycle->getNextDisjointTransposition();

        addTranspToDistMap(transp);

        uint elementCount = cycle->length();
        for(uint elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const word& element = (*cycle)[elementIndex];
            transpToCycleIndexMap[element] = cycleIndex;
        }
    }
}

word Generator::addTranspToDistMap(const Transposition& transp)
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

void Generator::sortDistanceKeys()
{
    //if(distKeys.size())
    //{
    //    word diff = distKeys.front();
    //    if(distMap.count(diff) && distMap[diff]->size() > 1)
    //    {
    //        return;
    //    }
    //}

    vector<DiffSortKey> keys(distMap.size());
    uint index = 0;

    forcin(iter, distMap)
    {
        DiffSortKey key;

        key.diff = iter->first;
        key.weight = countNonZeroBits(key.diff);
        ///key.length = (iter->second)->size();

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

        /*
        bool  isLeftGood = ( leftKey.length > 1);
        bool isRightGood = (rightKey.length > 1);

        bool isLess = false;
        if(isLeftGood == isRightGood)
        {
            isLess = leftKey.weight < rightKey.weight;
        }
        else
        {
            isLess = (isLeftGood && !isRightGood);
        }
        */

        bool isLess = (leftKey.capacity > rightKey.capacity);
        if(leftKey.capacity == rightKey.capacity)
        {
            isLess = leftKey.weight < rightKey.weight;
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

void Generator::sortCandidates(shared_ptr<list<Transposition>> candidates)
{
//    auto sortFunc = [=](Transposition left, Transposition right)
//    {
//        word leftTarget  = min(left. getX(), left. getY());
//        word rightTarget = min(right.getX(), right.getY());

//        uint leftWeight = countNonZeroBits(leftTarget);
//        uint rightWeight = countNonZeroBits(rightTarget);

//        bool isLess = (leftWeight < rightWeight);
//        return isLess;
//    };

//    candidates->sort(sortFunc);

    typedef struct { uint weight; Transposition transp; } CandSortKey;

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

tuple<Transposition, Transposition>
Generator::findBestCandidates(shared_ptr<list<Transposition>> candidates)
{
    uint candidateCount = candidates->size();
    assert(candidateCount > 1, string("Too few candidates"));

    Transposition first;
    Transposition firstPartner;
    uint firstDist;

    Transposition second;
    Transposition secondPartner;
    uint secondDist;

#if defined(ADDITIONAL_MEMORY_TECHNIQUE)
    first = candidates->front();
    if(first.getX() & 1)
    {
        first.swap();
    }

    second.setX(first.getX() ^ 2);
    second.setY(first.getY() ^ 2);
#else //ADDITIONAL_MEMORY_TECHNIQUE
    auto iter = candidates->begin();

    tie(firstPartner,  firstDist ) = findBestCandidatePartner(candidates, *iter);
    ++iter;

    tie(secondPartner, secondDist) = findBestCandidatePartner(candidates, *iter);

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
#endif //ADDITIONAL_MEMORY_TECHNIQUE

    return tie(first, second);
}

tuple<Transposition, uint>
Generator::findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
                                    const Transposition& target)
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

Transposition Generator::removeTranspFromPermutation(const Transposition& transp)
{
    word x = transp.getX();
    uint cycleIndex = transpToCycleIndexMap[x];

    auto cycle = permutation.getCycle(cycleIndex);
    cycle->remove(transp);

    Transposition temp;
    if(!cycle->isEmpty())
    {
        temp = cycle->getNextDisjointTransposition();
    }

    return temp;
}

void Generator::onCycleRemoved( uint cycleIndex )
{

}

void Generator::removeTranspFromDistMap(const Transposition& target)
{
    word diff = target.getDiff();
    auto& transpositions = distMap[diff];

    transpositions->remove(target);
    diffToEdgeMap.erase(diff);

    if(!transpositions->size())
    {
        distMap.erase(diff);
    }
}

deque<ReverseElement> Generator::implementCandidates(
    shared_ptr<list<Transposition>> candidates)
{
    word initialMask = 0;
    if(candidates && candidates->size())
    {
        const Transposition& transp = candidates->front();
        initialMask = transp.getDiff();
    }
    else
    {
        return deque<ReverseElement>();
    }

    BooleanEdge edge;
    if(diffToEdgeMap.count(initialMask))
    {
        edge = diffToEdgeMap[initialMask];
        diffToEdgeMap.erase(initialMask);
    }
    else
    {
        BooleanEdgeSearcher edgeSearcher(candidates, n, initialMask);
        edge = edgeSearcher.findEdge();
    }

    deque<ReverseElement> elements;
    if(edge.isValid() && edge.getCapacity() > 2)
    {
        if(edge.isFull())
        {
            word mask = 1;
            while(initialMask >= mask)
            {
                if(initialMask & mask)
                {
                    ReverseElement element(n, mask);
                    elements.push_back(element);
                }

                mask <<= 1;
            }

            shared_ptr<list<Transposition>> processedCandidates(new list<Transposition>);
            *processedCandidates = *candidates;

            reduceCandidatesCount(candidates, processedCandidates);
        }
        else
        {
            elements = implementCandidatesEdge(candidates, edge);
        }
    }
    else
    {
        elements = implementBestCandidatesPair(candidates, n);
    }

    return elements;
}

deque<ReverseElement> Generator::implementCandidatesEdge(
    shared_ptr<list<Transposition>> candidates, BooleanEdge edge)
{
    assert(candidates && candidates->size(), string("Invalid candidates passed to implementCandidatesEdge()"));

    word baseValue = edge.getBaseValue(n);
    word baseMask  = edge.getBaseMask(n);
    word diffMask  = candidates->front().getDiff();

    deque<ReverseElement> elements;
    word mask = 1;

    while(mask <= diffMask)
    {
        if(diffMask & mask)
        {
            ReverseElement element(n, mask, baseMask, ~baseValue & baseMask);
            elements.push_back(element);
        }

        mask <<= 1;
    }

    shared_ptr<list<Transposition>> processedCandidates =
        BooleanEdgeSearcher::getEdgeSubset(edge, n, candidates);
    reduceCandidatesCount(candidates, processedCandidates);

    return elements;
}

deque<ReverseElement> Generator::implementBestCandidatesPair(
    shared_ptr<list<Transposition>> candidates, uint n)
{
    if(!candidatesSorted)
    {
        sortCandidates(candidates);
        candidatesSorted = true;
    }

    TransposPair  transpPair;
    Transposition firstTransp;
    Transposition secondTransp;

    tie(firstTransp, secondTransp) = findBestCandidates(candidates);

    reduceCandidatesCount(candidates, firstTransp, false);
    reduceCandidatesCount(candidates, secondTransp);

    transpPair = TransposPair(firstTransp, secondTransp);
    transpPair.setN(n);

    ///auto elements = transpPair.getImplementation();
    deque<ReverseElement> elements;

    // implement first transposition
    auto firstElements = implementSingleTransposition(firstTransp);
    elements.insert(elements.end(), firstElements.cbegin(), firstElements.cend());

    // implement second transposition
    //secondTransp.swap();
    auto secondElements = implementSingleTransposition(secondTransp);
    elements.insert(elements.end(), secondElements.cbegin(), secondElements.cend());

    return elements;
}

void Generator::reduceCandidatesCount(
    shared_ptr<list<Transposition>> candidates,
    shared_ptr<list<Transposition>> processedCandidates)
{
    forin(iter, *processedCandidates)
    {
        reduceCandidatesCount(candidates, *iter, false);
    }

    word diff = distKeys.front();
    diffToEdgeMap.erase(diff);

    uint candidateCount = candidates->size();
    if(candidateCount < 2)
    {      
        if(!candidateCount)
        {
            distMap.erase(diff);
        }

        sortDistanceKeys();
        candidatesSorted = false;
    }
}

void Generator::reduceCandidatesCount(
    shared_ptr<list<Transposition>> candidates, const Transposition& transp,
    bool updateDistanceMapAndKeys /* = true */)
{
    candidates->remove(transp);
    removeTranspFromDistMap(transp);

    Transposition newTransp = removeTranspFromPermutation(transp);
    if(!newTransp.isEmpty())
    {
        addTranspToDistMap(newTransp);
    }

    if(updateDistanceMapAndKeys)
    {
        uint candidateCount = candidates->size();
        if(candidateCount < 2)
        {
            if(!candidateCount)
            {
                word diff = distKeys.front();
                distMap.erase(diff);
                diffToEdgeMap.erase(diff);
            }

            sortDistanceKeys();
            candidatesSorted = false;
        }
    }
}

deque<ReverseElement> Generator::implementCommonPair()
{
    // 1) find first
    word diff = distKeys.front();
    Transposition firstTransp = distMap[diff]->front();

    // 2) remove first
    word skipKeyValue;
    bool skipKeyFound = false;

    TransposPair  transpPair;
    Transposition newTransp = removeTranspFromPermutation(firstTransp);

    removeTranspFromDistMap(firstTransp);
    distKeys.remove(firstTransp.getDiff());

    distMap.erase(diff);
    diffToEdgeMap.erase(diff);
    distKeys.remove(diff);

    if(!newTransp.isEmpty())
    {
        diff = addTranspToDistMap(newTransp);
        if(distMap[diff]->size() > 1)
        {
            skipKeyValue = diff;
            skipKeyFound = true;
        }

        if(!found(distKeys, diff))
        {
            distKeys.push_back(diff);
        }
    }

    // 3) find second
    uint minComplexity = uintUndefined;
    Transposition secondTransp;

    forin(iter, distKeys)
    {
        const word& key = *iter;
        if(skipKeyFound && key == skipKeyValue)
        {
            continue;
        }

        Transposition transp = distMap[key]->front();
        TransposPair pair = TransposPair(firstTransp, transp);
        pair.setN(n);

        uint complexity = pair.getEstimateImplComplexity();
        if(minComplexity == uintUndefined || complexity < minComplexity)
        {
            minComplexity = complexity;
            secondTransp = transp;
            transpPair = TransposPair(firstTransp, transp);
        }
    }

    assert(!secondTransp.isEmpty(), string("Second transposition is empty"));

    // 4) remove second
    bool needSorting = skipKeyFound;
    newTransp = removeTranspFromPermutation(secondTransp);

    diff = secondTransp.getDiff();
    removeTranspFromDistMap(secondTransp);
    distKeys.remove(diff);

    distMap.erase(diff);
    diffToEdgeMap.erase(diff);
    distKeys.remove(diff);

    if(!newTransp.isEmpty())
    {
        diff = addTranspToDistMap(newTransp);
        if(distMap[diff]->size() > 1)
        {
            needSorting = true;
        }

        if(!found(distKeys, diff))
        {
            distKeys.push_back(diff);
        }
    }

    // 5) prepare for next iteration
    if(needSorting)
    {
        sortDistanceKeys();
        candidatesSorted = false;
    }

    transpPair.setN(n);

    auto elements = transpPair.getImplementation();
    return elements;
}

deque<ReverseElement> Generator::implementSingleTransposition(const Transposition& transp)
{
    /// New method: use maximum control inputs as possible
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
    ReverseElement element(n, targetMask, fullMask ^ targetMask,
        ~y & (fullMask ^ targetMask));
    elements.push_back(element);

    if(diff)
    {
        word value = y ^ targetMask;
        
        word mask = targetMask << 1;
        while(mask <= diff)
        {
            if(diff & mask)
            {
                ReverseElement element(n, mask, fullMask ^ mask,
                    ~value & (fullMask ^ mask));
                conjugationElements.push_back(element);
                
                value ^= mask;
            }

            mask <<= 1;
        }

        elements = conjugate(elements, conjugationElements, true);
    }

    return elements;
}

bool Generator::checkSchemeAgainstPermutationVector(const Scheme& scheme,
    const PermutationTable& table)
{
    bool result = true;
    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        const word& y = table[x];

        forin(iter, scheme)
        {
            const ReverseElement& element = *iter;
            x = element.getValue(x);
        }

        if(x != y)
        {
            result = false;
            break;
        }
    }

    return result;
}

void Generator::computeEdges()
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

BooleanEdge Generator::computeEdge(word diff, bool force /* = false */)
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

Generator::DiffSortKey::DiffSortKey()
    : diff(0)
    , length(uintUndefined)
    , weight(uintUndefined)
    , capacity(0)
//  , isGood(false)
{

}

}   // namespace ReversibleLogic
