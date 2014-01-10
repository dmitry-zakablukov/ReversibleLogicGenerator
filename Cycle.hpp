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

    void prepareForDisjoint(word diff = 0);
    shared_ptr<list<Transposition>> disjoint(bool isLeftMultiplication);

    shared_ptr<list<Transposition>> getBestTranspositionsForDisjoint();

    shared_ptr<Cycle> multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication) const;

    /// @targetElements - elements from transpositions (x and y)
    shared_ptr<Cycle> multiplyByTranspositions(const unordered_set<word>& targetElements,
        word diff, bool isLeftMultiplication) const;

protected:
    void disjoint(bool isLeftMultiplication, shared_ptr<list<Transposition>> transpositions,
        list<shared_ptr<Cycle>>* restCycles);

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

    DisjointParams findBestDisjointPoint(const DisjointParamsMap& disjointParams);

    bool hasDisjointPoint;
    DisjointParams bestParams;

    struct DisjointResult
    {
        shared_ptr<list<Transposition>> transpositions;
        list<shared_ptr<Cycle>> restCycles;
    };

    DisjointResult leftDisjointResult;
    DisjointResult rightDisjointResult;
};

}   // namespace ReversibleLogic
