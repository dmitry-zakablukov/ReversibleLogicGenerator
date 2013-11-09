#pragma once

namespace ReversibleLogic
{

class Generator
{
public:
    Generator();

    typedef vector<word> PermutationTable;
    typedef deque<ReverseElement> Scheme;

    Scheme generate(const PermutationTable& table, ostream& log);

    void checkPermutationValidity(const PermutationTable& table);

private:
    tuple<uint, Permutation> getPermutation(const PermutationTable& table);
    void prepareTranspToCycleIndexMap(uint n, const Permutation& permutation);

    map<word, shared_ptr<list<Transposition> >> getDistancesMap(const Permutation& permutation);

    word addTranspToDistMap(map<word, shared_ptr<list<Transposition> >>& distMap,
                            Transposition transp);

    list<word> getSortedDistanceKeys(map<word, shared_ptr<list<Transposition> >>& distMap);

    void sortCandidates(shared_ptr<list<Transposition>> candidates);

    tuple<Transposition, Transposition>
    findBestCandidates(shared_ptr<list<Transposition>>& candidates);

    tuple<Transposition, uint>
    findBestCandidatePartner(const shared_ptr<list<Transposition>>& candidates,
                             const Transposition& target);

    tuple<Transposition, Transposition>
    removeTranspFromPermutation(Permutation& permutation, Transposition& transp);
    void onCycleRemoved(uint cycleIndex);

    void removeTranspFromDistMap(map<word, shared_ptr<list<Transposition> >>& distMap,
                                 Transposition& target);

    bool checkSchemeAgainstPermutationVector(const Scheme& scheme,
                                             const PermutationTable& table);

    struct DiffSortKey;

    // optimization
    vector<uint> transpToCycleIndexMap;
};

struct Generator::DiffSortKey
{
    DiffSortKey();

    word diff;
    uint length;
    uint weight;
    //bool isGood = false;
};

}   // namespace ReversibleLogic
