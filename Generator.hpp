#pragma once

namespace ReversibleLogic
{

class Generator
{
public:
    Generator();
    virtual ~Generator() = default;

    Scheme generate(const TruthTable& table, ostream& outputLog);
    void checkPermutationValidity(const TruthTable& table);

private:
    tuple<uint, Permutation> getPermutation(const TruthTable& table);

    void implementPartialResult(PartialGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
        const TruthTable& table);

    shared_ptr<PartialGenerator> reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n;
    Permutation permutation;

    /// Results logging
    ostream* log;
};

}   // namespace ReversibleLogic
