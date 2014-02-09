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

    void setPermutation(const Permutation& thePermutation, uint inputCount);
    const Permutation& getPermutation() const;

    bool isLeftAndRightMultiplicationDiffers() const;

    void prepareForGeneration();
    PartialResultParams getPartialResultParams() const;

    /// Returns residual permutation, which would be left multiplied by partial result
    /// i.e. (partial_result) * (residual_permutation) if isLeftMultiplication == true
    /// and  (residual_permutation) * (partial_result) for isLeftMultiplication == false
    Permutation getResidualPermutation(bool isLeftMultiplication) const;

    // TODO: return shared_ptr
    deque<ReverseElement> implementPartialResult();

private:
    PartialResultParams getPartialResult(shared_ptr<list<Transposition>> transpositions,
        word diff, const PartialResultParams& bestParams);

    shared_ptr<list<Transposition>> findBestCandidates(shared_ptr<list<Transposition>> candidates);
    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, uint>
        findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
        const Transposition& target);

    deque<ReverseElement> implementEdge();
    deque<ReverseElement> implementPairOfTranspositions();
    deque<ReverseElement> implementSingleTransposition(const Transposition& transp);

    Permutation permutation;
    uint n;

    PartialResultParams partialResultParams;
};

} //namespace ReversibleLogic