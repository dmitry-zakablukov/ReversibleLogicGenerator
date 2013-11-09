#pragma once

namespace ReversibleLogic
{

class Generator
{
public:
    Generator();

    typedef vector<word> PermutationTable;
    typedef deque<ReverseElement> Scheme;

    Scheme generate(const PermutationTable& table, ostream& outputLog);
    void checkPermutationValidity(const PermutationTable& table);

private:
    tuple<uint, Permutation> getPermutation(const PermutationTable& table);
    void prepareTranspToCycleIndexMap();

    void fillDistancesMap();

    word addTranspToDistMap(Transposition& transp);
    void removeTranspFromDistMap(Transposition& target);

    void sortDistanceKeys();
    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, Transposition>
    findBestCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, uint>
    findBestCandidatePartner(const shared_ptr<list<Transposition>> candidates,
                             const Transposition& target);

    tuple<Transposition, Transposition>
    removeTranspFromPermutation(const Transposition& transp);
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
