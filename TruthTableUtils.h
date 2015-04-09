#pragma once

namespace ReversibleLogic
{

class TruthTableUtils
{
public:
    static TruthTable optimizeHammingDistance(const TruthTable& original, uint n, uint m,
        unordered_map<uint, uint>* outputVariablesOrder = 0);

private:
    /// Returns minimal number of input variables to make permutation from @original truth table
    static uint calculateNewInputVariableCount(const TruthTable& original, uint n, uint m);

    /// Returns new truth table with beginning part equals to the @original one
    static TruthTable extendTruthTable(const TruthTable &original, uint k, uint n);

    struct DistanceSum
    {
        DistanceSum() = default;

        int sum = 0;
        word index = 0;
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

    /// Returns mask where bit 1 is corresponding to one of output variables
    static word calculateOutputsMask(const unordered_map<uint, uint>& newOutputVariablesOrder);

    /// Picks the best outputs for @table inputs based on minimal Hamming distance
    static void pickUpBestOutputValues(TruthTable* table,
        uint n, uint k, word outputsMask);

    static void pickUpBestOutputValues(TruthTable* table, unordered_set<word> inputs,
        unordered_set<word> outputs);

    typedef unordered_map<word, deque<DistanceSum>> InputToBestIndexMap;

    /// Choose two best outputs for every input based on Hamming distance
    static void updateBestIndicesForInput(InputToBestIndexMap* map, const unordered_set<word>& inputs,
        const unordered_set<word>& outputs);
};

} //namespace ReversibleLogic
