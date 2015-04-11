#pragma once

namespace ReversibleLogic
{

class GtGenerator
{
public:
    GtGenerator() = default;
    virtual ~GtGenerator() = default;

    Scheme generate(const TruthTable& table, ostream& outputLog);

private:
    tuple<uint, Permutation> getPermutation(const TruthTable& table);
    void checkPermutationValidity(const TruthTable& table);

    void implementPartialResult(PartialGtGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    shared_ptr<PartialGtGenerator> reducePermutation(shared_ptr<PartialGtGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n = 0;
    Permutation permutation;
};

}   // namespace ReversibleLogic
