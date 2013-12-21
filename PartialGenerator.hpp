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

    void setPermutation(Permutation thePermutation, uint inputCount);
    void prepareForGeneration();

    PartialResultParams getPartialResultParams() const;

    /// Returns false if left and right multiplication by partial result
    /// would produce the same residual permutation
    bool isLeftAndRightMultiplicationDiffers() const;

    /// @isLeftMultiplication - parameter, for which true means that
    /// residual permutation would be left multiplied by partial result
    /// i.e. (partial_result) * (residual_permutation) for isLeftMultiplication == true
    /// and  (residual_permutation) * (partial_result) for isLeftMultiplication == false
    Permutation getResidualPermutation(bool isLeftMultiplication);

    // TODO: return shared_ptr
    deque<ReverseElement> implementPartialResult();

private:
    void fillDistancesMap();

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

    Permutation permutation;
    uint n;

    unordered_map<word, shared_ptr<list<Transposition>> > distMap;
    list<word> distKeys;

    unordered_map<word, uint> transpToCycleIndexMap;
    unordered_map<word, BooleanEdge> diffToEdgeMap;

    PartialResultParams partialResultParams;
    shared_ptr<list<Transposition>> transpositionsToSynthesize;
};

} //namespace ReversibleLogic