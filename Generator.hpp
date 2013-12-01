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

    void fillDistancesMap();

    word addTranspToDistMap(const Transposition& transp);
    void removeTranspFromDistMap(const Transposition& target);

    void sortDistanceKeys();
    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, Transposition>
    findBestCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, uint>
    findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
                             const Transposition& target);

    shared_ptr<list<Transposition>> removeTranspFromPermutation(const Transposition& transp);
    void onCycleRemoved(uint cycleIndex);

    // Edge optimization
    void computeEdges();
    BooleanEdge computeEdge(word diff, bool force = false);

    deque<ReverseElement> implementCandidates(
        shared_ptr<list<Transposition>> candidates);

    deque<ReverseElement> implementCandidatesEdge(
        shared_ptr<list<Transposition>> candidates, BooleanEdge edge);

    deque<ReverseElement> implementBestCandidatesPair(
        shared_ptr<list<Transposition>> candidates, uint n);

    void reduceCandidatesCount(shared_ptr<list<Transposition>> candidates,
        shared_ptr<list<Transposition>> processedCandidates);

    void reduceCandidatesCount(
        shared_ptr<list<Transposition>> candidates, const Transposition& transp,
        bool updateDistanceMapAndKeys = true);

    deque<ReverseElement> implementCommonPair();

    deque<ReverseElement> implementSingleTransposition(const Transposition& transp);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
        const PermutationTable& table);

    struct DiffSortKey;

    uint n;
    Permutation permutation;
    map<word, shared_ptr<list<Transposition>> > distMap;
    list<word> distKeys;
    bool candidatesSorted;

    // optimization
    vector<uint> transpToCycleIndexMap;
    unordered_map<word, BooleanEdge> diffToEdgeMap;

    // log
    ostream* log;
};

struct Generator::DiffSortKey
{
    DiffSortKey();

    word diff;
    uint length;
    uint weight;
    word capacity;
    //bool isGood = false;
};

}   // namespace ReversibleLogic
