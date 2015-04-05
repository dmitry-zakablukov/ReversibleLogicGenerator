#include "std.hpp"

namespace ReversibleLogic
{

//static
TruthTable TruthTableUtils::optimizeHammingDistance(const TruthTable& original, uint n, uint m)
{
    assertd(original.size() == (size_t)(1 << n),
        string("TruthTableUtils::optimizeHammingDistance(): invalid table size"));

    uint k = calculateNewInputVariableCount(original, n, m);
    TruthTable table = extendTruthTable(original, k, n);

    vector<SumVector> distances = calculateHammingDistances(table, k, n, m);
    sortSumVectorsForOutputVariables(&distances, n);

    unordered_map<uint, uint> newOutputVariablesOrder = calculateNewOrderOfOutputVariables(&distances, m);

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
            sumVector[index].columnIndex = index;
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
    vector<SumVector>* distances, uint m)
{
    unordered_map<uint, uint> newOrderMap;
    vector<SumVector>& distancesAlias = *distances;

    while (newOrderMap.size() != m)
    {
        // find first not processed index
        uint targetIndex = 0;
        while (newOrderMap.find(targetIndex) != newOrderMap.cend())
            ++targetIndex;

        // find conflicted output variables
        unordered_set<uint> samePositionIndices = { targetIndex };
        uint targetVariableIndex = distancesAlias[targetIndex].front().columnIndex;

        for (uint index = targetIndex + 1; index < m; ++index)
        {
            if (newOrderMap.find(index) != newOrderMap.cend())
                continue;

            const vector<DistanceSum>& dist = distancesAlias[index];

            // assuming that n > 1
            if (dist[0].columnIndex == targetVariableIndex)
            {
                if (dist[0].sum > dist[1].sum)
                    samePositionIndices.insert(index);
                else
                {
                    // remove this variable index from vector
                    auto first = distancesAlias[index].cbegin() + 1;
                    auto last = distancesAlias[index].cend();

                    vector<DistanceSum> newVector(first, last);
                    distancesAlias[index] = newVector;
                }
            }
        }

        if (samePositionIndices.size() == 1)
        {
            newOrderMap[targetIndex] = targetVariableIndex;
            continue;
        }
        else
        {
            // calculate total sum
            int totalSum = 0;
            for (uint index : samePositionIndices)
                totalSum += distancesAlias[index][1].sum;

            uint bestIndex = *(samePositionIndices.cbegin());
            uint maxSum = totalSum - distancesAlias[bestIndex][1].sum;

            // find best index
            for (auto iter = ++(samePositionIndices.cbegin()); iter != samePositionIndices.cend(); ++iter)
            {
                uint index = *iter;
                uint sum = totalSum - distancesAlias[index][1].sum;

                if (sum > maxSum)
                {
                    bestIndex = index;
                    maxSum = sum;
                }
            }

            // add best index to new order
            newOrderMap[bestIndex] = targetVariableIndex;

            // remove this variable index from other vectors
            for (auto index : samePositionIndices)
            {
                if (index == bestIndex)
                    continue;

                auto first = distancesAlias[index].cbegin() + 1;
                auto last = distancesAlias[index].cend();

                vector<DistanceSum> newVector(first, last);
                distancesAlias[index] = newVector;
            }
        }
    }

    return newOrderMap;
}

} //namespace ReversibleLogic
