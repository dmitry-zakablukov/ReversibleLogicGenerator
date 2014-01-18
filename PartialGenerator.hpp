#pragma once

namespace ReversibleLogic
{

/// Class for generating only part of resulting scheme
/// Used by Generator
class PartialGenerator
{
public:
    PartialGenerator();
    ~PartialGenerator();

    void setPermutation(Permutation* thePermutation, uint inputCount,
        bool isLeftMultiplication);
    void prepareForGeneration();

    PartialResultParams getPartialResultParams() const;

    /// Returns residual permutation, which would be left multiplied by partial result
    /// i.e. (partial_result) * (residual_permutation) if leftMultiplicationFlag == true
    /// and  (residual_permutation) * (partial_result) for leftMultiplicationFlag == false
    Permutation getResidualPermutation();

    // TODO: return shared_ptr
    deque<ReverseElement> implementPartialResult();

private:
    shared_ptr<list<Transposition>> getTranspositions();
    void fillDistancesMap(shared_ptr<list<Transposition>> transpositions);

    // Edge optimization
    void computeEdges();
    BooleanEdge computeEdge(word diff, bool force = false);

    void sortDistanceKeys();

    word addTranspToDistMap(const Transposition& transp);

    void processSameDiffTranspositions(shared_ptr<list<Transposition>> candidates);
    void processCommonTranspositions();

    void findBestCandidates(shared_ptr<list<Transposition>> candidates);
    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, uint>
        findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
        const Transposition& target);

    deque<ReverseElement> implementEdge();
    deque<ReverseElement> implementPairOfTranspositions();
    deque<ReverseElement> implementSingleTransposition(const Transposition& transp);

    Permutation* permutation;
    uint n;
    bool leftMultiplicationFlag;

    unordered_map<word, shared_ptr<list<Transposition>> > distMap;
    list<word> distKeys;

    unordered_map<word, BooleanEdge> diffToEdgeMap;

    PartialResultParams partialResultParams;
    shared_ptr<list<Transposition>> transpositionsToSynthesize;
};

} //namespace ReversibleLogic