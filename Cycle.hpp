#pragma once

namespace ReversibleLogic
{

class Cycle
{
public:
    Cycle();
    Cycle(vector<word>&& source);

    //Cycle& operator=(const Cycle& another) = default;

    // Finalize cycle: none element could be added to cycle
    void finalize();

    // Return true if cycle is finalized
    bool isFinal() const;

    // Add new element to cycle (before finalization)
    void append(word element);

    uint length() const;
    bool isEmpty() const;

    const word& operator[](uint index) const;
    Transposition getTranspositionByPosition(uint index) const;

    bool operator ==(const Cycle& another) const;
    bool has(const Transposition& target) const;

    // Returns true if there is some transposition in firstTranspositions
    bool remove(const Transposition& target);

    operator string() const;

    // Returns new value for input if it is in cycle
    // i.e. output = elements[i+1] if input == elements[i]
    word getOutput(word input) const;

    /// @frequencyMap - dictionary for calculation diff frequency throw all cycles
    void prepareForDisjoint(unordered_map<word, uint>* frequencyMap);

    /// @diff - target difference. If cycle hasn't this diff in disjointParams map,
    /// then the most frequent diff is choosing for disjoint 
    void setDisjointDiff(word diff);

    shared_ptr<list<Transposition>> disjoint(bool isLeftMultiplication,
        uint* restCyclesDistanceSum);

    // This method finds best pair for @transp in rest cycles
    // Should be called only if there is only one transposition in result
    void completeDisjoint(bool isLeftMultiplication,
        shared_ptr<list<Transposition>> result);

    uint getDistancesSum() const;

    shared_ptr<list<Transposition>> getBestTranspositionsForDisjoint();

    /// @targetElements - elements from transpositions (x and y)
    /// Returns list of new cycles if all transpositions from
    /// disjoint result are in multiplication and sets flag
    /// in @hasNewCycles in that case
    list<shared_ptr<Cycle>> multiplyByTranspositions(
        const unordered_set<word>& targetElements,
        bool isLeftMultiplication, bool* hasNewCycles);

protected:
    void prepareForDisjoint(word diff);

    void disjoint(bool isLeftMultiplication, shared_ptr<list<Transposition>> transpositions,
        list<shared_ptr<Cycle>>* restCycles);

    Transposition findBestPair(const Transposition& transp, uint* complexity);

private:
    /// Returns index modulo element count
    uint modIndex(uint index) const;
    uint modIndex(uint index, uint mod) const;

    // Calculates sum of Hamming distances for all cycle
    void calculateDistancesSum();

    word findBestDiff(unordered_map<word, uint> sums);
    void removeElement(vector<word>* target, uint index);
    shared_ptr<list<Transposition>> getTranspositionsByDiff(word diff);

    vector<word> elements;
    bool finalized;

    uint distnancesSum;
    uint newDistancesSum;

    shared_ptr<list<Transposition>> firstTranspositions;
    vector<word> restElements;

    // generator 4.0 optimization
    struct DisjointParams
    {
        uint index;     // index of element in cycle
        uint step;      // distance in elements between this element and another one
        word diff;      // difference between this element and another one

        int  leftSum;   // sum of new Hamming distances if this element will be disjoint point as left product
        int  rightSum;  // sum of new Hamming distances if this element will be disjoint point as right product
    };

    typedef unordered_map<word, vector<DisjointParams>> DisjointParamsMap;

    void fillDisjointParams(DisjointParamsMap* disjointParams, word targetDiff);
    void getAdditionalSumForDisjointPoint(uint index, uint step,
        DisjointParams* params);

    DisjointParams findBestDisjointPoint(const DisjointParamsMap& disjointParams, word diff);

    list<shared_ptr<Cycle>> getRestCycles(bool isLeftMultiplication);

    void getCyclesAfterMultiplication(bool isLeftMultiplication,
        shared_ptr<list<Transposition>> transpositions,
        const list<shared_ptr<Cycle>>& cycles, list<shared_ptr<Cycle>>* result);

    word getOutput(word input, shared_ptr<list<Transposition>> transpositions) const;
    word getOutput(word input, const list<shared_ptr<Cycle>>& cycles) const;

    bool hasDisjointPoint;
    DisjointParams bestParams;

    DisjointParamsMap disjointParams;

    struct DisjointResult
    {
        shared_ptr<list<Transposition>> transpositions;
        list<shared_ptr<Cycle>> restCycles;
        uint restCyclesDistanceSum;
    };

    DisjointResult leftDisjointResult;
    DisjointResult rightDisjointResult;

    enum MultiplicationType
    {
        mtNone = 0,         // cycle was not multiplied by any transpositions
        mtLeftMultiplied,   // cycle was left multiplied by transpositions
        mtRightMultiplied,  // cycle was right multiplied by transpositions
    };

    MultiplicationType previousMultiplicationType;
};

}   // namespace ReversibleLogic
