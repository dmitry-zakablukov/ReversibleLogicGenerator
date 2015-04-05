#pragma once

namespace ReversibleLogic
{

class TruthTableUtils
{
public:
    static TruthTable optimizeHammingDistance(const TruthTable& original, uint n, uint m);

private:
    /// Returns minimal number of input variables to make permutation from @original truth table
    static uint calculateNewInputVariableCount(const TruthTable& original, uint n, uint m);

    /// Returns new truth table with beginning part equals to the @original one
    static TruthTable extendTruthTable(const TruthTable &original, uint k, uint n);

    struct DistanceSum
    {
        DistanceSum() = default;

        int sum = 0;
        uint columnIndex = 0;
    };

    typedef vector<DistanceSum> SumVector;

    /// Calculates Hamming distance for every output function with every input variable
    static vector<SumVector> calculateHammingDistances(const TruthTable& table, uint k, uint n, uint m);

    /// Sorts distances sum
    static void sortSumVectorsForOutputVariables(vector<SumVector>* distances, uint n);

    /// Returns map for new order of output variables
    static unordered_map<uint, uint> calculateNewOrderOfOutputVariables(vector<SumVector>* distances, uint m);

    /// Reorders output variables in @table according to @newOrderMap
    static void reorderOutputVariables(TruthTable* table, const unordered_map<uint, uint>& newOrderMap,
        uint n, uint m);

    static word reorderBits(word x, uint bitCount, const unordered_map<uint, uint>& reorderMap);
};

} //namespace ReversibleLogic
