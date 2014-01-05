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

    // Get next transposition which will be first in decomposition
    // this transposition minimize sum of Hamming distances for all cycle
    Transposition getNextDisjointTransposition();

    shared_ptr<list<Transposition>> getBestTranspositionsForDisjoint();

    shared_ptr<Cycle> multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication) const;

    /// @targetElements - elements from transpositions (x and y)
    shared_ptr<Cycle> multiplyByTranspositions(const unordered_set<word>& targetElements,
        word diff, bool isLeftMultiplication) const;

private:
    /// Returns index modulo element count
    uint modIndex(uint index) const;
    uint modIndex(uint index, uint mod) const;

    // Find next best transposition,
    // which minimize sum of Hamming distances for all cycle
    void findBestDisjointIndex();

    // Calculates sum of Hamming distances for all cycle
    void calculateDistancesSum();

    word findBestDiff(unordered_map<word, uint> sums);
    void removeElement(vector<word>* target, uint index);
    shared_ptr<list<Transposition>> getTranspositionsByDiff(word diff);

    vector<word> elements;
    bool finalized;

    uint distnancesSum;
    uint newDistancesSum;

    Transposition disjointTransp;
    uint disjointIndex;

    shared_ptr<list<Transposition>> firstTranspositions;
    vector<word> restElements;
};

}   // namespace ReversibleLogic
