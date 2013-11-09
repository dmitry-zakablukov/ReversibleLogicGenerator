#pragma once

namespace ReversibleLogic
{

class Permutation
{
public:
    Permutation();
    explicit Permutation(vector<shared_ptr<Cycle>> theCycles);

    void append(shared_ptr<Cycle> cycle);
    //void remove(shared_ptr<Cycle> cycle);

    uint length() const;
    shared_ptr<Cycle> getCycle(uint index) const;

    bool isEmpty() const;

    bool isEven() const;

    void completeToEven();

    operator string();

    // range-based for loop
    vector<shared_ptr<Cycle>>::const_iterator begin() const;
    vector<shared_ptr<Cycle>>::const_iterator end() const;

private:
    vector<shared_ptr<Cycle>> cycles;
};

}   // namespace ReversibleLogic
