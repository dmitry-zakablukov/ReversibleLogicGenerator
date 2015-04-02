#pragma once

namespace ReversibleLogic
{

class Cycle
{
public:
    Cycle();
    Cycle(vector<word>&& source);

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
    bool has(word x) const;

    friend ostream& operator <<(ostream& out, const Cycle& cycle);

    // Returns new value for input if it is in cycle
    // i.e. output = elements[i+1] if input == elements[i]
    word getOutput(word input) const;

    /// @frequencyMap - dictionary for calculation diff frequency throw all cycles
    void prepareForDisjoint(unordered_map<word, uint>* frequencyMap);

    /// Finds all possible transpositions from cycle which have specified Hamming distance
    /// @diff - Hamming distance which should have all returned transpositions
    /// @result - output list for adding found transpositions
    void disjointByDiff(word diff, shared_ptr<list<Transposition>> result) const;

    void multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication, vector<shared_ptr<Cycle>>* output) const;

    uint getDistancesSum() const;

private:
    /// Returns index modulo element count
    uint modIndex(uint index) const;
    uint modIndex(uint index, uint mod) const;

    // generator 4.0 optimization
    word getOutput(word input, shared_ptr<list<Transposition>> transpositions) const;

    /// Gets all possible transpositions with specified Hamming distance @diff
    /// from vector of elements @input and puts them to @result
    void getTranspositionsByDiff(const vector<word>& input, word diff,
        shared_ptr<list<Transposition>> result) const;

    void getTranspositionsByDiff(const vector<word>& input, word diff,
        uint xIndex, uint yIndex, shared_ptr<list<Transposition>> result) const;

    vector<word> elements;
    bool finalized;
};

}   // namespace ReversibleLogic
