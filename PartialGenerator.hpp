#pragma once

namespace ReversibleLogic
{

/// Class for generating only part of resulting scheme
/// Used by Generator
class PartialGenerator
{
public:
    PartialGenerator(uint packSize = uintUndefined);
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

    deque<ReverseElement> implementPartialResult();

private:
    PartialResultParams getPartialResult(shared_ptr<list<Transposition>> transpositions,
        word diff, const PartialResultParams& bestParams);

    shared_ptr<list<Transposition>> getTranspositionsPack(const unordered_map<word, uint>& frequencyMap);
    void getTranspositionsPack(shared_ptr<list<Transposition>> result, Permutation* permCopy,
        unordered_set<word>* visited, bool reverseOrder);

    shared_ptr<list<Transposition>> getCommonPair();

    shared_ptr<list<Transposition>> findBestCandidates(shared_ptr<list<Transposition>> candidates);
    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, uint>
        findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
        const Transposition& target);

    deque<ReverseElement> implementEdge();
    deque<ReverseElement> implementPairOfTranspositions();
    deque<ReverseElement> implementSingleTransposition(const Transposition& transp);
    
    /// Implement multiple independent transpositions as one k-CNOT and many CNOT gates
    deque<ReverseElement> implementIndependentTranspositions(shared_ptr<list<Transposition>> transp);

    vector<word> transposeMatrix(const vector<word>& matrix, uint m) const;

    struct MatrixMix
    {
        vector<word> matrix; //new matrix after mix
        vector<word> columns; //new columns of this matrix
        unordered_map<uint, uint> columnIndexMap; //index mapping for original matrix
    };

    /// Removes copies of columns in matrix
    deque<ReverseElement> removeColumnsCopies(const vector<word>& transposedMatrix, uint k,
        MatrixMix* output, word* inversionMask) const;

    /// Columns with weight close to k / 2 go first
    MatrixMix reorderMatrixColumns(const MatrixMix& mix, uint k) const;

    /// Make canonical form of matrix
    deque<ReverseElement> transformMatrixToCanonicalForm(MatrixMix* mix, uint matrixWidth,
        word* inversionMask) const;

    void findBestRowInMatrix(const vector<word>& matrix, word pattern, word mask,
        uint* xIndex, uint* yIndex) const;

    deque<ReverseElement> transformRowToCanonicalForm(MatrixMix* mix, uint rowIndex,
        uint matrixWidth, uint baseVectorCount, word canonicalForm) const;

    word getRealMask(MatrixMix* mix, word inputMask) const;

    void applyModificationToMatrix(MatrixMix* mix, word controlMask, word targetMask) const;

    Permutation permutation;
    uint n;

    PartialResultParams partialResultParams;

    uint maxPackSize;
};

} //namespace ReversibleLogic