#include "std.hpp"

namespace ReversibleLogic
{

Generator::Generator()
    : transpToCycleIndexMap()
{
}

Generator::Scheme Generator::generate(const PermutationTable& table, ostream& log)
{
    float totalTime = 0;
    float time = 0;

    uint n = 0;
    Permutation permutation;

    {
        AutoTimer timer( &time );

        checkPermutationValidity(table);

        tie(n, permutation) = getPermutation(table);
        prepareTranspToCycleIndexMap(n, permutation);
    }

    log << "Permutation creation time: ";
    log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    log << " sec\n";

    totalTime += time;

    //debug
    //log << (string)permutation << "\n";

    Scheme scheme;
    map<word, shared_ptr<list<Transposition> >> distMap;
    list<word> distKeys;
    bool candidatesSorted = false;

    //debug
    //return scheme;

    time = 0;
    {
        AutoTimer timer( &time );

        distMap = getDistancesMap(permutation);
        distKeys= getSortedDistanceKeys(distMap);

        while(!permutation.isEmpty())
        {
            //debug
            log << (string)permutation << "\n";

            TransposPair nextPair;

            auto& candidates = distMap[distKeys.front()];
            if(candidates->size() > 1)
            {
                if(!candidatesSorted)
                {
                    sortCandidates(candidates);
                    candidatesSorted = true;
                }

                Transposition firstTransp;
                Transposition secondTransp;

                tie(firstTransp, secondTransp) = findBestCandidates(candidates);

                Transposition newTransp;
                Transposition oldTransp;

                tie(newTransp, oldTransp) =
                        removeTranspFromPermutation(permutation, firstTransp);
                candidates->remove(firstTransp);
                if(!newTransp.isEmpty())
                {
                    assert(!oldTransp.isEmpty(), string("Old transposition is empty"));

                    addTranspToDistMap(distMap, newTransp);
                    removeTranspFromDistMap(distMap, oldTransp);
                }

                tie(newTransp, oldTransp) =
                        removeTranspFromPermutation(permutation, secondTransp);
                candidates->remove(secondTransp);
                if(!newTransp.isEmpty())
                {
                    assert(!oldTransp.isEmpty(), string("Old transposition is empty"));

                    addTranspToDistMap(distMap, newTransp);
                    removeTranspFromDistMap(distMap, oldTransp);
                }

                uint candidateCount = candidates->size();
                if(candidateCount < 2)
                {
                    if(!candidateCount)
                    {
                        distMap.erase(distKeys.front());
                    }

                    distKeys = getSortedDistanceKeys(distMap);
                    candidatesSorted = false;
                }

                nextPair = TransposPair(firstTransp, secondTransp);
            }
            else
            {
                // 1) find first
                word diff = distKeys.front();
                Transposition firstTransp = distMap[diff]->front();

                // 2) remove first
                word skipKeyValue;
                bool skipKeyFound = false;

                Transposition newTransp;
                Transposition oldTransp;

                tie(newTransp, oldTransp) =
                        removeTranspFromPermutation(permutation, firstTransp);

                distMap.erase(diff);
                distKeys.remove(diff);

                if(!newTransp.isEmpty())
                {
                    assert(!oldTransp.isEmpty(), string("Old transposition is empty"));

                    diff = addTranspToDistMap(distMap, newTransp);
                    if(distMap[diff]->size() > 1)
                    {
                        skipKeyValue = diff;
                        skipKeyFound = true;
                    }
                    if(!found(distKeys, diff))
                    {
                        distKeys.push_back(diff);
                    }

                    removeTranspFromDistMap(distMap, oldTransp);
                    distKeys.remove( oldTransp.getDiff() );
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
                        nextPair = TransposPair(firstTransp, transp);
                    }
                }

                assert(!secondTransp.isEmpty(), string("Second transposition is empty"));

                // 4) remove second
                bool needSorting = skipKeyFound;
                tie(newTransp, oldTransp) =
                        removeTranspFromPermutation(permutation, secondTransp);

                diff = secondTransp.getDiff();
                distMap.erase(diff);
                distKeys.remove(diff);

                if(!newTransp.isEmpty())
                {
                    assert(!oldTransp.isEmpty(), string("Old transposition is empty"));

                    diff = addTranspToDistMap(distMap, newTransp);
                    if(distMap[diff]->size() > 1)
                    {
                        needSorting = true;
                    }
                    if(!found(distKeys, diff))
                    {
                        distKeys.push_back(diff);
                    }

                    removeTranspFromDistMap(distMap, oldTransp);
                    distKeys.remove( oldTransp.getDiff() );
                }

                // 5) prepare for next iteration
                if(needSorting)
                {
                    distKeys = getSortedDistanceKeys(distMap);
                    candidatesSorted = false;
                }
            }

            // 5)
            //debug
            log << (string)nextPair << "\n";
            nextPair.setN(n);

            auto elements = nextPair.getImplementation();
            scheme.insert(scheme.end(), elements.cbegin(), elements.cend());
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

    log << "Scheme synthesis time: ";
    log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    log << " sec\n";

    totalTime += time;

    time = 0;
    {
        AutoTimer timer( &time );

        log << "Complexity before optimization: " << scheme.size() << '\n';

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

        log << "Complexity after optimization: " << scheme.size() << '\n';

        bool isValid = checkSchemeAgainstPermutationVector(scheme, table);
        //////debug
        ////assert(isValid, string("Generated scheme is not valid"));
    }

    log << "Optimization time: ";
    log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    log << " sec\n";

    totalTime += time;

    log << "Total time: ";
    log << setiosflags(ios::fixed) << setprecision(2) << totalTime / 1000;
    log << " sec\n";

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

void Generator::prepareTranspToCycleIndexMap(uint n, const Permutation& permutation)
{
    word vectorSize = 1 << n;
    transpToCycleIndexMap.resize(vectorSize);

    uint cycleCount = permutation.length();
    for(uint cycleIndex = 0; cycleIndex < cycleCount; ++cycleIndex)
    {
        auto cycle = permutation.getCycle(cycleIndex);
        uint elementCount = cycle->length();
        for(uint elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const word& element = (*cycle)[elementIndex];
            transpToCycleIndexMap[element] = cycleIndex;
        }
    }
}

map<word, shared_ptr<list<Transposition> >> Generator::getDistancesMap(const Permutation& permutation)
{
    map<word, shared_ptr<list<Transposition>>> distMap;
    forin(iter, permutation)
    {
        const shared_ptr<Cycle>& cycle = *iter;

        uint elementCount = cycle->length();
        for(uint index = 0; index < elementCount; ++index)
        {
            Transposition transp = cycle->getTranspositionByPosition(index);
            addTranspToDistMap(distMap, transp);
        }
    }

    return distMap;
}

word Generator::addTranspToDistMap(map<word, shared_ptr<list<Transposition> >>& distMap,
                                   Transposition transp)
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
    }

    return diff;
}

list<word> Generator::getSortedDistanceKeys(map<word, shared_ptr<list<Transposition> >>& distMap)
{
    vector<DiffSortKey> keys(distMap.size());
    uint index = 0;

    forin(iter, distMap)
    {
        DiffSortKey key;

        key.diff = iter->first;
        key.weight = countNonZeroBits(key.diff);
        key.length = (iter->second)->size();
        //key.isGood = (key.length > 1);

        keys[index] = key;
        ++index;
    }

    auto sortFunc = [](const DiffSortKey& leftKey, const DiffSortKey& rightKey) -> bool
    {
        //bool isLess = leftKey.length > rightKey.length;
        bool isLess = (leftKey.length > 1 ) && (rightKey.length == 1);
        if(leftKey.length == rightKey.length)
        {
            isLess = leftKey.weight < rightKey.weight;
        }

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

        return isLess;
    };

    sort(keys.begin(), keys.end(), sortFunc);

    list<word> sortedKeys;
    forin(iter, keys)
    {
        const DiffSortKey& key = *iter;
        sortedKeys.push_back(key.diff);
    }

    return sortedKeys;
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
Generator::findBestCandidates(shared_ptr<list<Transposition>>& candidates)
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
Generator::findBestCandidatePartner(const shared_ptr<list<Transposition>>& candidates,
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

tuple<Transposition, Transposition>
Generator::removeTranspFromPermutation(Permutation& permutation,
                                       Transposition& transp)
{
    Transposition newTransp;
    Transposition oldTransp;

    word x = transp.getX();
    uint cycleIndex = transpToCycleIndexMap[x];

    auto cycle = permutation.getCycle(cycleIndex);
    cycle->remove(transp, &newTransp, &oldTransp);
    //if(cycle->isEmpty())
    //{
    //    permutation.remove(cycle);
    //}

    //forin(iter, permutation)
    //{
    //    const shared_ptr<Cycle>& cycle = *iter;
    //    if(cycle->has(transp))
    //    {
    //        cycle->remove(transp, &newTransp, &oldTransp);
    //        if(cycle->isEmpty())
    //        {
    //            permutation.remove(cycle);
    //        }

    //        break;
    //    }
    //}

    return tie(newTransp, oldTransp);
}

void Generator::onCycleRemoved( uint cycleIndex )
{

}

void Generator::removeTranspFromDistMap(map<word, shared_ptr<list<Transposition> >>& distMap,
                                        Transposition& target)
{
    word diff = target.getDiff();
    auto& transpositions = distMap[diff];

    // check if this needed
//    Transposition transp;
//    for(auto& t : transpositions)
//    {
//        if(t == target)
//        {
//            transp = t;
//            break;
//        }
//    }

//    assert(!transp.isEmpty(), string("Transposition not found"));

    transpositions->remove(target);
    if(!transpositions->size())
    {
        distMap.erase(diff);
    }
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


Generator::DiffSortKey::DiffSortKey()
    : diff(0)
    , length(uintUndefined)
    , weight(uintUndefined)
//  , isGood(false)  
{

}

}   // namespace ReversibleLogic
