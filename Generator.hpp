#pragma once

namespace ReversibleLogic
{

class Generator
{
public:
    Generator();

    Scheme generate(const PermutationTable& table, ostream& outputLog);
    void checkPermutationValidity(const PermutationTable& table);

private:
    tuple<uint, Permutation> getPermutation(const PermutationTable& table);

    void implementPartialResult(PartialGenerator* partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    void implementPartialResult(PartialGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
        const PermutationTable& table);

    // generator 4.0 optimization
    shared_ptr<PartialGenerator> reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    void prepareCyclesInPermutation(Permutation* permutation);

    /// Returns false if left and right multiplication by partial result
    /// would produce the same residual permutation
    bool isLeftAndRightMultiplicationDiffers(const Permutation* permutation) const;

    void reducePermutation(Permutation* permutation, uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n;
    Permutation permutation;

    // log
    ostream* log;
};

}   // namespace ReversibleLogic
