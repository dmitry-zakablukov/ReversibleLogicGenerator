#pragma once

namespace ReversibleLogic
{

class PostProcessor
{
public:
    PostProcessor();

    typedef vector<ReverseElement> OptScheme;
    OptScheme optimize(const OptScheme& scheme);

private:
    enum
    {
        numMaxElementCountInReplacements = 3,
    };

    struct OptimizationParams;
    typedef vector<OptimizationParams> Optimizations;

    void prepareSchemeForOptimization(const OptScheme& scheme, Optimizations* optimizations);
    OptScheme applyOptimizations(const OptScheme& scheme, const Optimizations& optimizations);

    // Optimize CNOTs and CCNOTs with inversions
    uint findInversedElementsSequence(const OptScheme& scheme, uint startPosition);
    OptScheme optimizeInversions(const OptScheme& scheme);

    // Remove duplicates elements
    OptScheme removeDuplicates(const OptScheme& scheme);

    // (01)(11) -> *1
    OptScheme mergeOptimization(OptScheme& scheme, bool* optimized);
    // (01)(10) -> (*1)(1*)
    OptScheme reduceConnectionsOptimization(OptScheme& scheme, bool* optimized);

    // Transfer optimization: two elements are swapped with producing new element
    OptScheme transferOptimization(OptScheme& scheme, bool* optimized);

    OptScheme getFullScheme(const OptScheme& scheme, bool heavyRight = true);
    OptScheme getFinalSchemeImplementation(const OptScheme& scheme);

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
    OptScheme tryOptimizationTactics(const OptScheme& scheme, SelectionFunc selectionFunc, SwapFunc swapFunc,
        bool* optimizationSucceeded, bool searchPairFromEnd,
        bool lessComplexityRequired, int* startIndex = 0);

    int getMaximumTransferIndex(const OptScheme& scheme, const ReverseElement& target,
        int startIndex, int stopIndex) const;

    /// Inserts replacements to result scheme based on original scheme
    void insertReplacements(const OptScheme& originalScheme,
        OptScheme* resultScheme,
        int leftIndex, int leftTransferedIndex,
        int rightIndex, int rightTransferedIndex,
        const list<ReverseElement>& leftReplacement,
        const list<ReverseElement>& rightReplacement);

    // Returns true if some duplicates were found in scheme and replacements
    bool processReplacements(const OptScheme& scheme,
        Optimizations* optimizations,
        int leftIndex, int leftTransferedIndex,
        int rightIndex, int rightTransferedIndex,
        const list<ReverseElement>& leftReplacement,
        const list<ReverseElement>& rightReplacement);

    void checkReplacement(const list<ReverseElement>& replacement);

    // Returns true if some duplicates were found in scheme and specified replacement
    bool processDuplicatesInReplacement(const OptScheme& scheme,
        Optimizations* optimizations,
        const list<ReverseElement>& replacement,
        const list<ReverseElement>* anotherReplacement,
        int originalIndex, int transferedIndex, bool searchToRight,
        list<ReverseElement>* processedReplacement);

    // Returns index of duplicate element in specified range of indices
    // or -1 if not found
    int findDuplicateElementIndex(const OptScheme& scheme, const ReverseElement& target,
        int startIndex, int stopIndex, int skipIndex) const;

    void setReplacement(const OptScheme& scheme, Optimizations* optimizations,
        list<ReverseElement>& replacement,
        int originalIndex, int transferedIndex);

    //OptimizationParams& getOptimization(uint index);
    //vector<OptimizationParams> optimizations;

    OptScheme testScheme;

    // this flag turns on and off second transfer optimization
    // inside transfer optimization, if less scheme complexity
    // is required
    bool secondPassOptimizationFlag;
};

struct PostProcessor::OptimizationParams
{
    OptimizationParams();

    bool inversions;
    bool heavyRight;
    // Remove flag suppress "as is" flag
    bool remove;
    bool replace;
    vector<ReverseElement> replacement;
    bool asis;
};

}   // ReversibleLogic