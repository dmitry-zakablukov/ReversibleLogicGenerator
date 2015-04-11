#pragma once

namespace ReversibleLogic
{

class GtGenerator
{
public:
    GtGenerator();
    virtual ~GtGenerator() = default;

    Scheme generate(const TruthTable& table, ostream& outputLog);
    void checkPermutationValidity(const TruthTable& table);

private:
    tuple<uint, Permutation> getPermutation(const TruthTable& table);

    void implementPartialResult(PartialGtGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    shared_ptr<PartialGtGenerator> reducePermutation(shared_ptr<PartialGtGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n;
    Permutation permutation;

    /// Results logging
    ostream* log;
};

}   // namespace ReversibleLogic
