#pragma once

// Simple struct for representing boolean edge
struct BooleanEdge
{
    BooleanEdge();

    /// Returns true, if edge is like **...*
    bool isFull() const;

    /// Returns true, if edge is valid
    /// By default edge is constructed as not valid
    bool isValid() const;

    word getCapacity() const;

    word getBaseMask(uint n) const;
    word getBaseValue(uint n) const;

    word baseValue;
    word starsMask;
    bool full;
};

// Class for searching boolean edge in set of binary vectors
class BooleanEdgeSearcher
{
public:
    explicit BooleanEdgeSearcher(shared_ptr<list<ReversibleLogic::Transposition>> inputSet,
        uint n, word initialMask);
    ~BooleanEdgeSearcher();

    BooleanEdge findEdge();
    shared_ptr<list<ReversibleLogic::Transposition>> getEdgeSubset(BooleanEdge edge, uint n); 

    static shared_ptr<list<ReversibleLogic::Transposition>> getEdgeSubset(BooleanEdge edge, uint n,
        shared_ptr<list<ReversibleLogic::Transposition>> transpositions);

private:
    void validateInputSettings();

    /// Returns max number of * in edge
    uint findMaxEdgeDimension(uint length);

    BooleanEdge findEdge(word edgeMask, uint restPositionCount, uint startPos);

    /// Returns true, if edge cover subset of input set
    bool checkEdge(BooleanEdge* edge);

    shared_ptr<list<ReversibleLogic::Transposition>> inputSet;
    uint n;
    word initialMask;

    vector<uint> frequencyTable;
};