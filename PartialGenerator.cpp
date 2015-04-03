#include "std.hpp"

namespace ReversibleLogic
{

PartialGenerator::PartialGenerator(uint packSize /*= uintUndefined*/)
    : permutation()
    , n(uintUndefined)
    , maxPackSize(packSize)
{
    if (maxPackSize == uintUndefined)
        maxPackSize = ProgramOptions::get().transpositionsPackSize;

    assert(countNonZeroBits(maxPackSize) == 1,
        string("Transpositions pack size should be power of 2"));
}

PartialGenerator::~PartialGenerator()
{
}

void PartialGenerator::setPermutation(const Permutation& thePermutation, uint inputCount)
{
    permutation = thePermutation;
    n = inputCount;

    assertd(n != uintUndefined, string("PartialGenerator: input count not defined"));
    partialResultParams.edge.n = n;
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

        bool isResultComparisonNeeded = false;
        if (ProgramOptions::get().isTuningEnabled)
            isResultComparisonNeeded = ProgramOptions::get().options.getBool(
                "compare-results-on-edge-search");

        if ((isResultComparisonNeeded && !bestResult.isBetterThan(result)) ||
            result.edge.coveredTranspositionCount > bestResult.edge.coveredTranspositionCount)
        {
            bestResult = result;
        }
    }

    if (bestResult.type != PartialResultParams::tFullEdge &&
        bestResult.type != PartialResultParams::tEdge)
    {
        // try to retrieve transpositions pack
        shared_ptr<list<Transposition>> transpositions = getTranspositionsPack(frequencyMap);
    
        if (transpositions->size())
        {
            bestResult.type = PartialResultParams::tPack;
            bestResult.transpositions = transpositions;
            bestResult.params.packSize = transpositions->size();
        }
    }

    if(bestResult.type == PartialResultParams::tNone)
    {
        shared_ptr<list<Transposition>> transpositions = getCommonPair();

        assertd(transpositions->size() > 1,
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

shared_ptr<list<Transposition>> PartialGenerator::getTranspositionsPack(const unordered_map<word, uint>& frequencyMap)
{
    // we should obtain no more than @maxPackSize transpositions
    // if there are not enough transpositions, result list size should be power of two

    bool reversOrder = true;
    if (ProgramOptions::get().isTuningEnabled)
        reversOrder = ProgramOptions::get().options.getBool(
        "transpositions-pack-in-reverse-order", reversOrder);

    unordered_set<word> visited;

    shared_ptr<list<Transposition>> result(new list<Transposition>);
    shared_ptr<list<Transposition>> temp(new list<Transposition>);

    Permutation permCopy = permutation.clone();
    bool stopFlag = false;

    for (auto iter : frequencyMap)
    {
        word diff = iter.first;
        temp->resize(0);
        
        for (auto cycle : permCopy)
            cycle->disjointByDiff(diff, temp);

        temp->remove_if(
            [&](const Transposition& t)
            {
                return visited.find(t.getX()) != visited.cend() ||
                    visited.find(t.getY()) != visited.cend();
            }
        );

        stopFlag = temp->size() == 0;
        if (stopFlag)
            break;

        // it really doesn't matter what multiplication is (left or right)
        // but this is only because we trying to obtain independent transpositions
        permCopy = permCopy.multiplyByTranspositions(temp, true); 

        for (auto& t : *temp)
        {
            if (reversOrder)
                result->push_front(t);
            else
                result->push_back(t);

            visited.insert(t.getX());
            visited.insert(t.getY());
        }

        if (result->size() >= maxPackSize)
            break;
    }

    if (result->size() > maxPackSize)
        result->resize(maxPackSize);
    else
    {
        // trying to get maximum possible number 
        getTranspositionsPack(result, &permCopy, &visited, reversOrder);

        uint size = result->size();

        // make from @size power of 2
        word maxSize = maxPackSize;
        while (maxSize > size)
            maxSize >>= 1;

        assert(maxSize, string("getTranspositionsPack(): error with calculation of result max size"));
        result->resize(maxSize);
    }

    return result;
}

void PartialGenerator::getTranspositionsPack(shared_ptr<list<Transposition>> result,
    Permutation* permCopy, unordered_set<word>* visited, bool reverseOrder)
{
    uint maxSize = maxPackSize - result->size();
    if (!maxSize)
        return;

    list<Transposition> temp;
        
    for (auto cycle : *permCopy)
    {
        uint pos = 0;
        word buffer[2] = {}; //for x and y

        uint elementCount = cycle->length();
        for (uint index = 0; index < elementCount; ++index)
        {
            word element = (*cycle)[index];
            if (visited->find(element) == visited->cend())
            {
                buffer[pos++] = element;
                visited->insert(element);
            }

            if (pos == 2)
                break;
        }

        if (pos == 2)
        {
            temp.push_back(Transposition(buffer[0], buffer[1]));
        }
    }

    uint tempSize = temp.size();
    if (tempSize == 0)
        return;

    if (tempSize > maxSize)
        temp.resize(maxSize);

    if (reverseOrder)
        result->insert(result->begin(), temp.crbegin(), temp.crend());
    else
        result->insert(result->end(), temp.cbegin(), temp.cend());

    if (result->size() < maxPackSize)
    {
        *permCopy = permCopy->multiplyByTranspositions(temp, true);
        getTranspositionsPack(result, permCopy, visited, reverseOrder);
    }
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
            word x = cycle[0];
            word y = cycle[1];

            transpositions->push_back(Transposition(x, y));

            uint mask = 1;
            while (true)
            {
                word a = x ^ mask;
                word b = y ^ mask;

                if (!cycle.has(a) && !cycle.has(b))
                {
                    transpositions->push_back(Transposition(a, b));
                    break;
                }

                mask <<= 1;
            }

            assert(transpositions->size() == 2, string("wrong representation of 3-cycle"));
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
    assertd(candidateCount > 1, string("PartialGenerator: too few candidates for findBestCandidates()"));

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

    assertd(!second.isEmpty(), string("Candidate partner not found"));
    return tie(second, minDist);
}


ReversibleLogic::Permutation PartialGenerator::getResidualPermutation(bool isLeftMultiplication) const
{
    return permutation.multiplyByTranspositions(partialResultParams.transpositions, isLeftMultiplication);
}

deque<ReverseElement> PartialGenerator::implementPartialResult()
{
    assertd(partialResultParams.transpositions->size(), string("PartialGenerator: no transpositions to synthesize"));

    deque<ReverseElement> synthesisResult;
    switch(partialResultParams.type)
    {
    case PartialResultParams::tFullEdge:
    case PartialResultParams::tEdge:
        synthesisResult = implementEdge();
        break;

    case PartialResultParams::tPack:
        synthesisResult = implementIndependentTranspositions(partialResultParams.transpositions);
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

    word baseValue = edge.getBaseValue();
    word baseMask  = edge.getBaseMask();

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
    assertd(partialResultParams.transpositions->size() == 2,
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
    bool targetInB10 = (x & targetMask) != 0; //(x_i == 1, y_i == 0)
    word base = (targetInB10 ? y : x);

    word mainElementInversionMask = ~(base ^ targetMask);
    mainElementInversionMask &= fullMask;

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

deque<ReverseElement> PartialGenerator::implementIndependentTranspositions(shared_ptr<list<Transposition>> transp)
{
    deque<ReverseElement> elements;

    uint k = transp->size() * 2;
    assert(k <= countNonZeroBits((word)-1),
        string("implementIndependentTranspositions(): too many rows"));

    assertd(countNonZeroBits(k) == 1,
        string("implementIndependentTranspositions(): only power of 2 is allowed for vector count"));

    uint baseVectorCount = (uint)(log(k) / log(2));

    // make matrix
    vector<word> matrix;
    matrix.reserve(k);

    for (const Transposition& t : *transp)
    {
        matrix.push_back(t.getX());
        matrix.push_back(t.getY());
    }

    // transpose matrix
    vector<word> transposedMatrix = transposeMatrix(matrix, n);

    // remove columns copies
    MatrixMix mix;
    word inversionMask = 0;

    {
        auto conjugationElements = removeColumnsCopies(transposedMatrix, k, &mix, &inversionMask);
        elements.insert(elements.cend(), conjugationElements.cbegin(), conjugationElements.cend());
    }

    // reorder columns
    uint matrixWidth = mix.columns.size();
    mix = reorderMatrixColumns(mix, k);

    // transform matrix to canonical form
    {
        auto conjugationElements = transformMatrixToCanonicalForm(&mix, matrixWidth, &inversionMask);
        elements.insert(elements.cend(), conjugationElements.cbegin(), conjugationElements.cend());
    }

    // conjugate core element
    {
        word controlMask = (1 << n) - 1;
        for (uint index = 0; index < baseVectorCount; ++index)
            controlMask ^= 1 << mix.columnIndexMap[index];

        ReverseElement element(n, 1 << mix.columnIndexMap[0], controlMask, inversionMask);
        elements = conjugate(deque<ReverseElement>{ element }, elements);
    }

    debugBehavior("PartialGenerator::implementIndependentTranspositions()-check-validity", [&]()->void
    {
        unordered_map<word, word> table;
        for (uint index = 0; index < ((uint)1 << n); ++index)
        {
            word x = index;
            word y = x;
        
            for (const ReverseElement& element : elements)
                y = element.getValue(y);
        
            if (y != x)
                table[x] = y;
        }
        
        assertd(table.size() == k,
            string("implementIndependentTranspositions(): validity check failed (count)"));
        
        for (auto& t : *transp)
        {
            word x = t.getX();
            word y = t.getY();
        
            assertd(table[x] == y,
                string("implementIndependentTranspositions(): validity check failed (x)"));
        
            assertd(table[y] == x,
                string("implementIndependentTranspositions(): validity check failed (y)"));
        }
    });

    return elements;
}

vector<word> PartialGenerator::transposeMatrix(const vector<word>& matrix, uint m) const
{
    vector<word> transposed;
    transposed.reserve(m);

    uint k = matrix.size();
    word mask = 1;

    for (uint index = 0; index < m; ++index)
    {
        word column = 0;
        uint pos = 1;

        for (auto row : matrix)
        {
            if (row & mask)
                column |= pos;

            pos <<= 1;
        }

        transposed.push_back(column);
        mask <<= 1;
    }

    return transposed;
}

deque<ReverseElement> PartialGenerator::removeColumnsCopies(const vector<word>& transposedMatrix, uint k,
    MatrixMix* output, word* inversionMask) const
{
    assertd(output && inversionMask, string("removeColumnsCopies(): null ptr"));

    // 1) find unique columns
    uint columnCount = transposedMatrix.size();
    unordered_map<word, list<uint>> columnToIndicesMap;

    for (uint index = 0; index < columnCount; ++index)
    {
        list<uint>& indices = columnToIndicesMap[transposedMatrix[index]];
        indices.push_back(index);
    }

    // 2) remove copies
    deque<ReverseElement> elements;
    unordered_set<word> visited;

    word mask = (1 << k) - 1;
    for (const auto& iter : columnToIndicesMap)
    {
        word column = iter.first;
        if (visited.find(column) != visited.cend())
            continue;

        if (column == mask)
            continue; //do nothing

        const list<uint>& indices = iter.second;

        if (column == 0)
        {
            for (auto index : indices)
                *inversionMask |= 1 << index;

            continue;
        }

        // complementary column
        word complementaryColumn = ~column & mask;
        if (columnToIndicesMap.find(complementaryColumn) != columnToIndicesMap.cend())
        {
            const list<uint>& complementaryIndices = columnToIndicesMap[complementaryColumn];
            visited.insert(complementaryColumn);

            for (auto citer = ++indices.cbegin(); citer != indices.cend(); ++citer)
            {
                ReverseElement element(n, 1 << *citer, 1 << *(complementaryIndices.cbegin()));
                elements.push_back(element);
            }

            for (auto citer = complementaryIndices.cbegin();
                citer != complementaryIndices.cend(); ++citer)
            {
                ReverseElement element(n, 1 << *citer, 1 << *(indices.cbegin()));
                elements.push_back(element);
            }
        }
        else
        {
            for (auto citer = ++indices.cbegin(); citer != indices.cend(); ++citer)
            {
                ReverseElement element(n, 1 << *citer, 1 << *(indices.cbegin()));
                elements.push_back(element);

                *inversionMask |= 1 << *citer;
            }
        }

        output->columns.push_back(column);
        output->columnIndexMap[output->columns.size() - 1] = *(indices.cbegin());

        visited.insert(column);
    }

    return elements;
}

PartialGenerator::MatrixMix PartialGenerator::reorderMatrixColumns(const MatrixMix& mix, uint k) const
{
    uint m = mix.columns.size();

    MatrixMix outputMix;
    outputMix.columns.reserve(m);

    struct Key
    {
        uint index;
        int dist;
    };

    vector<Key> keys;
    keys.reserve(m);

    uint index = 0;
    for (auto column : mix.columns)
    {
        uint weight = countNonZeroBits(column);
        int dist = abs((int)(k / 2 - weight));

        keys.push_back( { index, dist } );
        ++index;
    }

    auto sortFunc = [](const Key& leftKey, const Key& rightKey)
    {
        return leftKey.dist < rightKey.dist;
    };

    sort(keys.begin(), keys.end(), sortFunc);

    // make new columns
    for (uint index = 0; index < m; ++index)
    {
        uint columnIndex = keys[index].index;

        outputMix.columns.push_back(mix.columns[columnIndex]);
        outputMix.columnIndexMap[index] = mix.columnIndexMap.at(columnIndex);
    }

    // make matrix from columns
    outputMix.matrix = transposeMatrix(outputMix.columns, k);

    return outputMix;
}

deque<ReverseElement> PartialGenerator::transformMatrixToCanonicalForm(MatrixMix* mix, uint matrixWidth,
    word* inversionMask) const
{
    assertd(mix && inversionMask, string("transformMatrixToCanonicalForm(): null ptr"));

    deque<ReverseElement> elements;

    uint k = mix->matrix.size();
    assertd(countNonZeroBits(k) == 1,
        string("transformMatrixToCanonicalForm(): only power of 2 is allowed for vector count"));

    uint baseVectorCount = (uint)(log(k) / log(2));
    assert(baseVectorCount <= matrixWidth, string("transformMatrixToCanonicalForm(): wrong base vector count"));

    if (baseVectorCount == matrixWidth)
    {
        // bad case, no temp storage
        uint firstInversionPos = findPositiveBitPosition(*inversionMask);
        if (firstInversionPos == uintUndefined)
        {
            // the worst case, find first column not from mix->columns
            unordered_set<uint> forbiddenIndices;
            for (auto iter : mix->columnIndexMap)
                forbiddenIndices.insert(iter.second);

            for (uint index = 0; index < n; ++index)
            {
                if (forbiddenIndices.find(index) != forbiddenIndices.cend())
                    continue;

                elements.push_back(ReverseElement(n, 1 << index));
                firstInversionPos = index;
                
                break;
            }

            assert(firstInversionPos != uintUndefined,
                string("transformMatrixToCanonicalForm(): temp column not found"));
        }
        else
        {
            *inversionMask ^= 1 << firstInversionPos;
        }

        // add temporary column
        mix->columnIndexMap[matrixWidth] = firstInversionPos;

        ++matrixWidth;
    }

    // transformation itself
    uint baseMask = (1 << baseVectorCount) - 1;
    for (uint index = 0; index < k; index += 2)
    {
        uint xIndex = wordUndefined;
        uint yIndex = wordUndefined;

        findBestRowInMatrix(mix->matrix, index, baseMask, &xIndex, &yIndex);
        assertd(xIndex == (yIndex ^ 1), string("transformMatrixToCanonicalForm(): wrong indices found"));

        // transform x
        {
            auto transformElements = transformRowToCanonicalForm(mix, xIndex, matrixWidth,
                baseVectorCount, index);
            elements.insert(elements.cend(), transformElements.cbegin(), transformElements.cend());
        }

        // transform y
        {
            auto transformElements = transformRowToCanonicalForm(mix, yIndex, matrixWidth,
                baseVectorCount, index + 1);
            elements.insert(elements.cend(), transformElements.cbegin(), transformElements.cend());
        }
    }

    for (uint index = baseVectorCount; index < matrixWidth; ++index)
    {
        uint realIndex = mix->columnIndexMap[index];
        *inversionMask |= 1 << realIndex;
    }

    return elements;
}

void PartialGenerator::findBestRowInMatrix(const vector<word>& matrix, word pattern, word mask,
    uint* xIndex, uint* yIndex) const
{
    assertd(xIndex && yIndex, string("findBestRowInMatrix(): null ptr"));

    uint xBestIndex = uintUndefined;
    uint yBestIndex = uintUndefined;

    word xBest = wordUndefined;
    word yBest = wordUndefined;

    uint minDist = uintUndefined;

    uint rowCount = matrix.size();
    assertd((rowCount & 1) == 0, string("findBestRowInMatrix(): wrong row count"));

    for (uint index = 0; index < rowCount; index += 2)
    {
        uint xInd = index;
        uint yInd = index + 1;

        if (matrix[xInd] < pattern)
            continue; //skip rows in canonical form

        word x = (matrix[xInd] ^ pattern) & mask;
        word y = (matrix[yInd] ^ pattern) & mask;

        uint xWeight = countNonZeroBits(x);
        uint yWeight = countNonZeroBits(y);
        
        uint dist = xWeight + yWeight;
        if (dist > minDist)
            continue;

        if (yWeight < xWeight || ((x & 1) == 1 && (y & 1) == 0))
        {
            swap(x, y);
            swap(xWeight, yWeight);
            swap(xInd, yInd);
        }

        if (dist < minDist || xBest > x)
        {
            minDist = dist;

            xBest = x;
            yBest = y;

            xBestIndex = xInd;
            yBestIndex = yInd;
        }
    }

    assertd(xBestIndex != uintUndefined && yBestIndex != uintUndefined,
        string("findBestRowInMatrix(): row not found"));

    *xIndex = xBestIndex;
    *yIndex = yBestIndex;
}

deque<ReverseElement> PartialGenerator::transformRowToCanonicalForm(MatrixMix* mix, uint rowIndex,
    uint matrixWidth, uint baseVectorCount, word canonicalForm) const
{
    assertd(mix, string("transformRowToCanonicalForm(): null ptr"));
    assertd(rowIndex < mix->matrix.size(), string("transformRowToCanonicalForm(): invalid index parameter"));

    word baseMask = (1 << baseVectorCount) - 1;

    word row = mix->matrix[rowIndex];
    word baseDiff = (canonicalForm ^ row) & baseMask;
    word outerDiff = row & ~baseMask;

    // 1) check if row is already in canonical form
    if (!baseDiff && !outerDiff)
        return deque<ReverseElement>();

    deque<ReverseElement> elements;

    // 2) find non-zero element not form basis columns
    uint firstNonZeroElementPos = findPositiveBitPosition(outerDiff, baseVectorCount);
    if (firstNonZeroElementPos == uintUndefined)
    {
        // 3) make non-zero element from first non-basis column
        firstNonZeroElementPos = baseVectorCount;

        word targetMask = 1 << firstNonZeroElementPos;
        word controlMask = row & baseMask;

        ReverseElement element(n, getRealMask(mix, targetMask), getRealMask(mix, controlMask));
        elements.push_back(element);

        applyModificationToMatrix(mix, controlMask, targetMask);
    }

    // 4) make almost canonical row
    {
        word diff = baseDiff ^ outerDiff;
        if (diff & (1 << firstNonZeroElementPos))
            diff ^= 1 << firstNonZeroElementPos;

        uint mask = 1;
        while (mask <= diff)
        {
            if (diff & mask)
            {
                word targetMask = mask;
                word controlMask = 1 << firstNonZeroElementPos;

                ReverseElement element(n, getRealMask(mix, targetMask), getRealMask(mix, controlMask));
                elements.push_back(element);

                applyModificationToMatrix(mix, controlMask, targetMask);
            }

            mask <<= 1;
        }
    }

    // 5) remove non-zero element not form basis columns (== make canonical form)
    {
        word targetMask = 1 << firstNonZeroElementPos;
        word controlMask = canonicalForm;

        ReverseElement element(n, getRealMask(mix, targetMask), getRealMask(mix, controlMask));
        elements.push_back(element);

        applyModificationToMatrix(mix, controlMask, targetMask);
    }

    assertd(mix->matrix[rowIndex] == canonicalForm,
        string("transformRowToCanonicalForm(): transformation of row to canonical form is invalid"));

    return elements;
}

void PartialGenerator::applyModificationToMatrix(MatrixMix* mix, word controlMask, word targetMask) const
{
    assertd(mix, string("applyModificationToMatrix(): null ptr"));
    assertd(countNonZeroBits(targetMask) == 1,
        string("applyModificationToMatrix(): invalid target mask parameter"));

    for (auto& row : mix->matrix)
    {
        if ((row & controlMask) == controlMask)
            row ^= targetMask;
    }
}

word PartialGenerator::getRealMask(MatrixMix* mix, word inputMask) const
{
    word realMask = 0;

    word mask = 1;
    word index = 0;

    while (mask <= inputMask)
    {
        if (inputMask & mask)
            realMask |= 1 << (mix->columnIndexMap[index]);

        mask <<= 1;
        ++index;
    }

    return realMask;
}

} //namespace ReversibleLogic