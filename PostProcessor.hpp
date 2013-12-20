#pragma once

namespace ReversibleLogic
{

class PostProcessor
{
public:
    PostProcessor();

    typedef vector<ReverseElement> Scheme;
    Scheme optimize(const Scheme& scheme);

private:
    void prepareSchemeForOptimization(const Scheme& scheme);
    Scheme applyOptimizations(const Scheme& scheme);

    // Optimize CNOTs and CCNOTs with inversions
    uint findInversedElementsSequence(const Scheme& scheme, uint startPosition);
    Scheme optimizeInversions(const Scheme& scheme);

    // Remove duplicates elements
    Scheme removeDuplicates(const Scheme& scheme);

    // (01)(11) -> *1
    Scheme mergeOptimization(Scheme& scheme, bool* optimized = 0 );
    // (01)(10) -> (*1)(1*)
    Scheme reduceConnectionsOptimization(Scheme& scheme, bool* optimized = 0 );

    Scheme transferOptimization(Scheme& scheme, bool* optimized = 0);

    Scheme getFullScheme(const Scheme& scheme, bool heavyRight = true);
    Scheme getFinalSchemeImplementation(const Scheme& scheme);

    // Selection function should return true if and only if right element fits to
    // right element in terms of optimization
    typedef bool (*SelectionFunc)(const ReverseElement& left, const ReverseElement& right);

    // Swap function should return in leftReplacement list of elements for replacement after
    // left and right element would be swapped. Same for rightReplacement list.
    // left  element are definitely not in  leftReplacement and may be in rigthReplacement
    // right element are definitely not in rightReplacement and may be in  leftReplacement
    typedef void (*SwapFunc)(const ReverseElement& left, const ReverseElement& right,
        list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement);

    // Returns true in optimizationSucceeded param if optimization tactics
    // succeeded and result scheme has less gate complexity
    Scheme tryOptimizationTactics(const Scheme& scheme, SelectionFunc selectionFunc, SwapFunc swapFunc,
        bool* optimizationSucceeded, bool searchPairFromEnd,
        bool lessComplexityRequired, int* startIndex = 0);

    int getMaximumTransferIndex(const Scheme& scheme, const ReverseElement& target,
        int startIndex, int stopIndex) const;

    // Returns true if some duplicates were found in scheme and replacements
    bool processReplacements(const Scheme& scheme,
        int leftIndex, int transferedLeftIndex,
        int rightIndex, int transferedRightIndex,
        const list<ReverseElement>& leftReplacement,
        const list<ReverseElement>& rightReplacement);

    void checkReplacement(const list<ReverseElement>& replacement);

    // Returns true if some duplicates were found in scheme and specified replacement
    bool processDuplicatesInReplacement(const Scheme& scheme,
        const list<ReverseElement>& replacement,
        int originalIndex, int transferedIndex, bool searchToRight,
        list<ReverseElement>* processedReplacement);

    // Returns index of duplicate element in specified range of indices
    // or -1 if not found
    int findDuplicateElementIndex(const Scheme& scheme, const ReverseElement& target,
        int startIndex, int stopIndex, int skipIndex) const;

    void setReplacement(const Scheme& scheme, list<ReverseElement>& replacement,
        int originalIndex, int transferedIndex);

    struct Optimizations;
    Optimizations& getOptimization(uint index);

    vector<Optimizations> optimizations;
};

struct PostProcessor::Optimizations
{
    Optimizations();

    bool inversions;
    bool heavyRight;
    // Remove flag suppress "as is" flag
    bool remove;
    bool replace;
    vector<ReverseElement> replacement;
    bool asis;
};

}   // ReversibleLogic