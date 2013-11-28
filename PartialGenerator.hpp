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

    Permutation permutation;
    uint n;

    unordered_map<word, shared_ptr<list<Transposition>> > distMap;
    list<word> distKeys;

    unordered_map<word, uint> transpToCycleIndexMap;
    unordered_map<word, BooleanEdge> diffToEdgeMap;

    PartialResult partialResult;
    shared_ptr<list<Transposition>> resultTranspositions;
};

} //namespace ReversibleLogic