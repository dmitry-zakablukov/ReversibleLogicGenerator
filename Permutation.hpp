#pragma once

namespace ReversibleLogic
{

class Permutation
{
public:
    Permutation();
    explicit Permutation(vector<shared_ptr<Cycle>> theCycles);
    virtual ~Permutation() = default;

    void append(shared_ptr<Cycle> cycle);

    uint length() const;
    shared_ptr<Cycle> getCycle(uint index) const;

    uint getElementCount() const;
    uint getTranspositionsCount() const;

    bool isEmpty() const;

    bool isEven() const;

    void completeToEven();
    void completeToEven(word truthTableSize);

    friend ostream& operator <<(ostream& out, const Permutation& permutation);

    /// For range-based for loop
    vector<shared_ptr<Cycle>>::const_iterator begin() const;
    vector<shared_ptr<Cycle>>::const_iterator end() const;

    Permutation multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication) const;

    Permutation multiplyByTranspositions(const list<Transposition>& transpositions,
        bool isLeftMultiplication) const;

    uint getDistancesSum() const;

    Permutation clone() const;

private:
    vector<shared_ptr<Cycle>> cycles;
};

}   // namespace ReversibleLogic
