#pragma once

namespace ReversibleLogic
{

class Generator
{
public:
    Generator();

    Scheme generate(const TruthTable& table, ostream& outputLog);
    void checkPermutationValidity(const TruthTable& table);

private:
    tuple<uint, Permutation> getPermutation(const TruthTable& table);

    void implementPartialResult(PartialGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
        const TruthTable& table);

    // generator 4.0 optimization
    shared_ptr<PartialGenerator> reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n;
    Permutation permutation;

    // log
    ostream* log;
};

}   // namespace ReversibleLogic
