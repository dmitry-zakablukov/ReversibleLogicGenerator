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

    shared_ptr<PartialGenerator> reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
        const Permutation& permutation, uint n, Scheme* scheme, Scheme::iterator* targetIter);

    bool isLeftChoiceBetter(const PartialResultParams& leftPartialResultParams,
        const PartialResultParams& rightPartialResultParams);

    void implementPartialResult(shared_ptr<PartialGenerator> partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
        const PermutationTable& table);

    uint n;
    Permutation permutation;

    // log
    ostream* log;
};

}   // namespace ReversibleLogic
