#include "std.hpp"

namespace ReversibleLogic
{

//static
TruthTable TruthTableUtils::optimizeHammingDistance(const TruthTable& original, uint n, uint m,
    unordered_map<uint, uint>* outputVariablesOrder /*= 0*/)
{
    assertd(original.size() == (size_t)(1 << n),
        string("TruthTableUtils::optimizeHammingDistance(): invalid table size"));

    uint k = calculateNewInputVariableCount(original, n, m);
    TruthTable table = extendTruthTable(original, k, n);

    vector<SumVector> distances = calculateHammingDistances(table, k, n, m);
    sortSumVectorsForOutputVariables(&distances, n);

    unordered_map<uint, uint> newOutputVariablesOrder = calculateNewOrderOfOutputVariables(&distances, m);
    reorderOutputVariables(&table, newOutputVariablesOrder, n, m);

    word outputsMask = calculateOutputsMask(newOutputVariablesOrder);
    pickUpBestOutputValues(&table, n, k, outputsMask);

    makePermutationEvenIfNecessary(&table, k, n);

    if (outputVariablesOrder)
        *outputVariablesOrder = newOutputVariablesOrder;

    return table;
}

//static
uint TruthTableUtils::calculateNewInputVariableCount(const TruthTable& original, uint n, uint m)
{
    vector<uint> frequency;
    frequency.resize(1 << m);
    memset(frequency.data(), 0, sizeof(uint) * frequency.size());

    for (auto y : original)
        frequency[y] += 1;

    uint maxFrequency = frequency.front();
    for (auto freq : frequency)
    {
        if (freq > maxFrequency)
            maxFrequency = freq;
    }

    uint ancillaryInputCount = (uint)(ceil(log(maxFrequency) / log(2)));
    return max(n, m + ancillaryInputCount);
}

//static
TruthTable TruthTableUtils::extendTruthTable(const TruthTable &original, uint k, uint n)
{
    TruthTable table;
    if (k == n)
        table = original;
    else
    {
        table.resize(1 << k);
        memset(table.data(), 0, sizeof(TruthTable::value_type) * table.size());

        uint entryCount = original.size();
        for (uint index = 0; index < entryCount; ++index)
            table[index] = original[index];
    }

    return table;
}

//static
vector<TruthTableUtils::SumVector> TruthTableUtils::calculateHammingDistances(
    const TruthTable& table, uint k, uint n, uint m)
{
    vector<SumVector> distances;
    distances.resize(m);

    for (vector<DistanceSum>& sumVector : distances)
    {
        sumVector.resize(k);
        for (uint index = 0; index < k; ++index)
            sumVector[index].index = (word)index;
    }

    word count = (word)1 << n;
    for (word x = 0; x < count; ++x)
    {
        word y = table[x];

        for (uint outIndex = 0; outIndex < m; ++outIndex)
        {
            vector<DistanceSum>& sumVector = distances[outIndex];

            word temp = x;
            for (uint inIndex = 0; inIndex < k; ++inIndex)
            {
                DistanceSum& targetSum = sumVector[inIndex];
                targetSum.sum += (temp ^ y) & 1;

                temp >>= 1;
            }

            y >>= 1;
        }
    }

    return distances;
}

//static
void TruthTableUtils::sortSumVectorsForOutputVariables(vector<SumVector>* distances, uint n)
{
    int averageSum = 1 << (n - 1);
    for (auto& sumVector : *distances)
    {
        for (auto& dist : sumVector)
            dist.sum = abs(dist.sum - averageSum);
    }

    auto sortFunc = [=](const DistanceSum& left, const DistanceSum& right) -> bool
    {
        return left.sum > right.sum;
    };

    for (auto& sumVector : *distances)
        sort(sumVector.begin(), sumVector.end(), sortFunc);
}

//static
unordered_map<uint, uint> TruthTableUtils::calculateNewOrderOfOutputVariables(
    vector<SumVector>* distancesPtr, uint m)
{
    unordered_map<uint, uint> newOrderMap;
    vector<SumVector>& distances = *distancesPtr;

    // tuning option
    const ProgramOptions& options = ProgramOptions::get();
    bool chooseOutputOnlyByHammingDistance =
        options.options.getBool("choose-output-order-only-by-hamming-distance", false);

    while (newOrderMap.size() != m)
    {
        // find first not processed index
        uint targetIndex = 0;
        while (newOrderMap.find(targetIndex) != newOrderMap.cend())
            ++targetIndex;

        // find conflicted output variables
        unordered_set<uint> samePositionIndices = { targetIndex };
        word targetVariableIndex = distances[targetIndex].front().index;

        for (uint index = targetIndex + 1; index < m; ++index)
        {
            if (newOrderMap.find(index) != newOrderMap.cend())
                continue;

            const vector<DistanceSum>& dist = distances[index];

            // assuming that n > 1
            if (dist[0].index == targetVariableIndex)
                samePositionIndices.insert(index);
        }

        if (samePositionIndices.size() == 1)
        {
            newOrderMap[targetIndex] = targetVariableIndex;
        }
        else
        {
            uint bestIndex = *(samePositionIndices.cbegin());

            if (chooseOutputOnlyByHammingDistance)
            {
                int maxSum = distances[bestIndex][0].sum;

                // find best index
                for (auto iter = ++(samePositionIndices.cbegin()); iter != samePositionIndices.cend(); ++iter)
                {
                    uint index = *iter;
                    int sum = distances[index][0].sum;

                    if (sum > maxSum) //greater is better
                    {
                        bestIndex = index;
                        maxSum = sum;
                    }
                }
            }
            else // !chooseOutputOnlyByHammingDistance
            {
                // calculate total sum
                int totalSum = 0;
                for (uint index : samePositionIndices)
                    totalSum += distances[index][1].sum;

                uint maxSum = totalSum - distances[bestIndex][1].sum;

                // find best index
                for (auto iter = ++(samePositionIndices.cbegin()); iter != samePositionIndices.cend(); ++iter)
                {
                    uint index = *iter;
                    uint sum = totalSum - distances[index][1].sum;

                    if (sum > maxSum) //greater is better
                    {
                        bestIndex = index;
                        maxSum = sum;
                    }
                }
            }
            // add best index to new order
            newOrderMap[bestIndex] = targetVariableIndex;
        }

        // remove this variable index from other vectors
        for (auto& sumVector : distances)
        {
            for (auto iter = sumVector.begin(); iter != sumVector.end();)
            {
                if (iter->index == targetVariableIndex)
                    iter = sumVector.erase(iter);
                else
                    ++iter;
            }
        }
    }

    return newOrderMap;
}

//static
void TruthTableUtils::reorderOutputVariables(TruthTable* tablePtr,
    const unordered_map<uint, uint>& newOrderMap, uint n, uint m)
{
    TruthTable& table = *tablePtr;
    uint entryCount = (uint)1 << n;

    for (uint index = 0; index < entryCount; ++index)
    {
        word y = table[index];
        table[index] = reorderBits(y, m, newOrderMap);
    }
}

word TruthTableUtils::reorderBits(word x, uint bitCount, const unordered_map<uint, uint>& reorderMap)
{
    word y = 0;
    for (uint index = 0; index < bitCount; ++index)
        y |= ((x >> index) & 1) << reorderMap.at(index);

    return y;
}

//static
word TruthTableUtils::calculateOutputsMask(const unordered_map<uint, uint>& newOutputVariablesOrder)
{
    word mask = 0;

    for (auto iter : newOutputVariablesOrder)
        mask |= (word)1 << iter.second;

    return mask;
}

//static
void TruthTableUtils::pickUpBestOutputValues(TruthTable* tablePtr,
    uint n, uint k, word outputsMask)
{
    TruthTable& table = *tablePtr;

    word totalInputCount  = (word)1 << n;
    word totalOutputCount = (word)1 << k;

    // input entries
    unordered_set<word> unvisitedInputs;
    unvisitedInputs.reserve(totalInputCount);

    for (word value = 0; value < totalInputCount; ++value)
        unvisitedInputs.insert(value);

    // output entries
    unordered_set<word> unvisitedOutputs;
    unvisitedOutputs.reserve(totalOutputCount);

    for (word value = 0; value < totalOutputCount; ++value)
        unvisitedOutputs.insert(value);

    // main loop
    while (unvisitedInputs.size())
    {
        word x = *(unvisitedInputs.cbegin());
        word y = table[x];

        unordered_set<word> matchedInputs;
        matchedInputs.reserve(unvisitedInputs.size());

        for (auto input : unvisitedInputs)
        {
            if (table[input] == y)
                matchedInputs.insert(input);
        }

        unordered_set<word> matchedOutputs;
        matchedOutputs.reserve(unvisitedOutputs.size());

        word baseValue = y & outputsMask;
        for (auto output : unvisitedOutputs)
        {
            if ((output & outputsMask) == baseValue)
                matchedOutputs.insert(output);
        }

        pickUpBestOutputValues(tablePtr, matchedInputs, matchedOutputs);

        for (auto input : matchedInputs)
        {
            unvisitedInputs.erase(input);
            unvisitedOutputs.erase(table[input]);
        }
    }

    // pick up values for the rest of the table
    unvisitedInputs.reserve(totalOutputCount - totalInputCount);

    for (word value = totalInputCount; value < totalOutputCount; ++value)
        unvisitedInputs.insert(value);

    pickUpBestOutputValues(tablePtr, unvisitedInputs, unvisitedOutputs);
}

//static
void TruthTableUtils::pickUpBestOutputValues(TruthTable* tablePtr, unordered_set<word> inputs,
    unordered_set<word> outputs)
{
    TruthTable& table = *tablePtr;

    // tuning option
    const ProgramOptions& options = ProgramOptions::get();
    bool pickUpBestOutputOnlyByHammingDistance =
        options.options.getBool("pick-up-best-output-only-by-hamming-distance", false);

    InputToBestIndexMap inputToBestIndexMap;
    deque<DistanceSum> emptyDeque;

    for (auto input : inputs)
        inputToBestIndexMap[input] = emptyDeque;

    while (inputs.size())
    {
        if (outputs.size() == 1)
        {
            assert(inputs.size() == 1,
                string("TruthTableUtils::pickUpBestOutputValues(): not all outputs were assigned correctly"));

            word input  = *inputs.cbegin();
            word output = *outputs.cbegin();

            table[input] = output;
            break;
        }

        updateBestIndicesForInput(&inputToBestIndexMap, inputs, outputs);

        word first = *(inputs.cbegin());
        word bestOutput = inputToBestIndexMap[first].front().index;

        word bestInput = first;
        if (pickUpBestOutputOnlyByHammingDistance)
        {
            // choose best index
            int minSum = inputToBestIndexMap[first].front().sum;
            for (auto input : inputs)
            {
                deque<DistanceSum>& sum = inputToBestIndexMap[input];
                if (sum.front().index == bestOutput)
                {
                    int currentSum = sum.front().sum;
                    if (currentSum < minSum)
                    {
                        minSum = currentSum;
                        bestInput = input;
                    }
                }
            }
        }
        else // !pickUpBestOutputOnlyByHammingDistance
        {
            // find other inputs with the same best pair index
            int totalSum = 0;
            for (auto input : inputs)
            {
                deque<DistanceSum>& sum = inputToBestIndexMap[input];

                if (sum.front().index == bestOutput)
                    totalSum += sum.back().sum;
            }

            // choose best index
            int minSum = totalSum - inputToBestIndexMap[first].back().sum;

            for (auto input : inputs)
            {
                deque<DistanceSum>& sum = inputToBestIndexMap[input];

                if (sum.front().index == bestOutput)
                {
                    int currentSum = totalSum - sum.back().sum;
                    if (currentSum < minSum)
                    {
                        minSum = currentSum;
                        bestInput = input;
                    }
                }
            }
        }

        // assign output for best input
        table[bestInput] = bestOutput;

        // clear map
        for (auto iter : inputToBestIndexMap)
        {
            deque<DistanceSum>& sum = inputToBestIndexMap[iter.first];
            
            if (sum.front().index == bestOutput)
                sum.pop_front();

            if (sum.back().index == bestOutput)
                sum.pop_back();
        }

        inputToBestIndexMap.erase(bestInput);

        // clear inputs and outputs
        inputs.erase(bestInput);
        outputs.erase(bestOutput);
    }
}

//static
void TruthTableUtils::updateBestIndicesForInput(InputToBestIndexMap* mapPtr,
    const unordered_set<word>& inputs, const unordered_set<word>& outputs)
{
    InputToBestIndexMap& map = *mapPtr;

    for (auto iter : map)
    {
        word input = iter.first;
        deque<DistanceSum>& distances = map[input];

        if (distances.size() > 1)
            continue;

        for (auto output : outputs)
        {
            DistanceSum sum;
            sum.index = output;

            sum.sum = countNonZeroBits(input ^ output);

            // insert in sorted deque
            auto iter = distances.cbegin();
            bool isInserted = false;

            while (iter != distances.cend())
            {
                if (sum.index == iter->index)
                {
                    isInserted = true;
                    break;
                }
                else if (sum.sum < iter->sum)
                    break;

                ++iter;
            }

            if (!isInserted)
                distances.insert(iter, sum);

            if (distances.size() > 2)
                distances.pop_back();
        }
    }
}

//static
void TruthTableUtils::makePermutationEvenIfNecessary(TruthTable* tablePtr, uint k, uint n)
{
    const ProgramOptions& options = ProgramOptions::get();
    if (k > n && options.isTuningEnabled && options.options.getBool("complete-permutation-to-even", false))
    {
        TruthTable& table = *tablePtr;

        Permutation permutation = PermutationUtils::createPermutation(table, false);
        if (!permutation.isEven())
        {
            uint totalInputCount = (uint)1 << k;
            uint originalInputCount = (uint)1 << n;

            uint firstIndex = originalInputCount;
            uint lastIndex  = firstIndex + 1;

            int distanceChange = calculateHammingDistanceChangeForTwoEntriesSwap(table,
                firstIndex, lastIndex);

            for (uint outerIndex = firstIndex; outerIndex < totalInputCount; ++outerIndex)
            {
                for (uint innerIndex = outerIndex + 1; innerIndex < totalInputCount; ++innerIndex)
                {
                    int diff = calculateHammingDistanceChangeForTwoEntriesSwap(table,
                        outerIndex, innerIndex);

                    if (diff > distanceChange)
                    {
                        distanceChange = diff;
                        firstIndex = outerIndex;
                        lastIndex = innerIndex;
                    }
                }
            }

            swap(table[firstIndex], table[lastIndex]);
        }
    }
}

//static
int TruthTableUtils::calculateHammingDistanceChangeForTwoEntriesSwap(const TruthTable& table,
    uint firstIndex, uint lastIndex)
{
    int beforeSum =
        (int)countNonZeroBits(firstIndex ^ table[firstIndex]) +
        (int)countNonZeroBits(lastIndex ^ table[lastIndex]);

    int afterSum =
        (int)countNonZeroBits(firstIndex ^ table[lastIndex]) +
        (int)countNonZeroBits(lastIndex ^ table[firstIndex]);

    return beforeSum - afterSum;
}

} //namespace ReversibleLogic
