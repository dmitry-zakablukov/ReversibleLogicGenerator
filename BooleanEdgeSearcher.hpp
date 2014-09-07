#pragma once

// Simple struct for representing boolean edge
struct BooleanEdge
{
    BooleanEdge(uint n);

    /// Returns true, if edge is like **...*
    bool isFull() const;

    /// Returns true, if edge is valid
    /// By default edge is constructed as not valid
    bool isValid() const;

    bool has(word x) const;

    word getCapacity() const;

    word getBaseMask() const;
    word getBaseValue() const;

    uint n = uintUndefined;
    word baseValue;
    word starsMask;
    bool full;

    uint coveredTranspositionCount;
};

// Class for searching boolean edge in set of binary vectors
class BooleanEdgeSearcher
{
public:
    explicit BooleanEdgeSearcher(shared_ptr<list<ReversibleLogic::Transposition>> input,
        uint n, word initialMask);

    explicit BooleanEdgeSearcher(const unordered_set<word>& inputs, uint n);

    ~BooleanEdgeSearcher() = default;

    void setExplicitEdgeFlag(bool value);

    BooleanEdge findEdge();

    static shared_ptr<list<ReversibleLogic::Transposition>> filterTranspositionsByEdge(BooleanEdge edge, uint n,
        shared_ptr<list<ReversibleLogic::Transposition>> transpositions);

    shared_ptr<list<ReversibleLogic::Transposition>> getEdgeSubset(BooleanEdge edge, uint n);

    shared_ptr<unordered_set<word>> getEdgeSet(BooleanEdge edge);

private:
    void validateInputSettings();

    /// Returns max number of * in edge
    uint findMaxEdgeDimension(uint length);

    void findEdge(BooleanEdge* bestEdge, word edgeMask, uint restPositionCount, uint startPos);

    /// Returns true, if edge cover subset of input set
    bool checkEdge(BooleanEdge* edge);

    unordered_set<word> inputSet;
    uint n;
    word initialMask;

    bool explicitEdgeFlag = false;

    vector<uint> frequencyTable;
};