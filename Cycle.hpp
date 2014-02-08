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

    bool operator ==(const Cycle& another) const;
    bool has(const Transposition& target) const;

    operator string() const;

    // Returns new value for input if it is in cycle
    // i.e. output = elements[i+1] if input == elements[i]
    word getOutput(word input) const;

    /// @frequencyMap - dictionary for calculation diff frequency throw all cycles
    void prepareForDisjoint(unordered_map<word, uint>* frequencyMap);

    /// Finds all possible transpositions from cycle which have specified Hamming distance
    /// @diff - Hamming distance which should have all returned transpositions
    /// @result - output list for adding found transpositions
    void disjointByDiff(word diff, shared_ptr<list<Transposition>> result);

    // This method finds best pair for @transp in rest cycles
    // Should be called only if there is only one transposition in result
    void completeDisjoint(bool isLeftMultiplication,
        shared_ptr<list<Transposition>> result);

    /// @targetElements - elements from transpositions (x and y)
    /// Returns list of new cycles if all transpositions from
    /// disjoint result are in multiplication and sets flag
    /// in @hasNewCycles in that case
    list<shared_ptr<Cycle>> multiplyByTranspositions(
        const unordered_set<word>& targetElements,
        bool isLeftMultiplication, bool* hasNewCycles);

    void multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication, vector<shared_ptr<Cycle>>* output) const;

private:
    /// Returns index modulo element count
    uint modIndex(uint index) const;
    uint modIndex(uint index, uint mod) const;

    /// Returns best pair transposition for target transposition from cycle
    Transposition findBestPair(const Transposition& transp, uint* complexity);

    vector<word> elements;
    bool finalized;

    // generator 4.0 optimization
    list<shared_ptr<Cycle>> getRestCycles(bool isLeftMultiplication);

    void getCyclesAfterMultiplication(bool isLeftMultiplication,
        shared_ptr<list<Transposition>> transpositions,
        const list<shared_ptr<Cycle>>& cycles, list<shared_ptr<Cycle>>* result);

    word getOutput(word input, shared_ptr<list<Transposition>> transpositions) const;
    word getOutput(word input, const list<shared_ptr<Cycle>>& cycles) const;

    /// Gets all possible transpositions with specified Hamming distance @diff
    /// from vector of elements @input and puts them to @result
    void getTranspositionsByDiff(const vector<word>& input, word diff,
        shared_ptr<list<Transposition>> result);

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
