#pragma once

namespace ReversibleLogic
{

class PostProcessor
{
public:
    PostProcessor() = default;
    virtual ~PostProcessor() = default;

    typedef vector<ReverseElement> OptScheme;
    OptScheme optimize(const OptScheme& scheme);

    Scheme optimize(const Scheme& scheme);

private:
    enum
    {
        numMaxElementCountInReplacements = 3,
    };

    struct OptimizationParams;
    typedef vector<OptimizationParams> Optimizations;

    // Selection function should return true if and only if right element fits to
    // right element in terms of optimization
    typedef bool(*SelectionFunc)(const ReverseElement& left, const ReverseElement& right);

    // Swap function should return in leftReplacement list of elements for replacement after
    // left and right element would be swapped. Same for rightReplacement list.
    // left  element are definitely not in  leftReplacement and may be in rigthReplacement
    // right element are definitely not in rightReplacement and may be in  leftReplacement
    typedef void(*SwapFunc)(const ReverseElement& left, const ReverseElement& right,
        list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement);

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

    // General optimization function for merge, reduce and transfer optimization
    OptScheme generalOptimization(OptScheme& scheme, bool* optimized,
        SelectionFunc selectFunc, SwapFunc swapFunc,
        bool searchPairFromEnd, bool lessComplexityRequired);

    enum FullSchemeType
    {
        fstSimple,
        fstRecursive,
        fstToffoli,
    };

    OptScheme getFullScheme(const OptScheme& scheme, FullSchemeType type, bool heavyRight = true);
    OptScheme getFinalSchemeImplementation(const OptScheme& scheme);

    // Returns true in optimizationSucceeded param if optimization tactics
    // succeeded and result scheme has less gate complexity
    OptScheme tryOptimizationTactics(const OptScheme& scheme, SelectionFunc selectionFunc, SwapFunc swapFunc,
        bool* optimizationSucceeded, bool searchPairFromEnd,
        bool lessComplexityRequired, int* startIndex = 0);

    // First element - new reverse element obtained as a result of swapping
    // Second element - range of indices, on which this element is freely swappable
    typedef pair<ReverseElement, Range> SwapResult;

    // Returns swap result for element on @startIndex position in @scheme.
    // Elements in @scheme will be changed accordingly to swap operations, so make backup before using.
    // Param @toLeft determines, where this element is moving.
    deque<SwapResult> getSwapResult(OptScheme* scheme, uint startIndex, bool toLeft);

    deque<SwapResult> mergeSwapResults(deque<SwapResult>& toLeft, deque<SwapResult>& toRight);

    // First element of pair - swap results for left element
    // Second element of pair - swap results for right element
    typedef struct 
    {
        deque<SwapResult> forLeft;
        deque<SwapResult> forRight;
    } SwapResultsPair;
        
    SwapResultsPair getSwapResultsPair(const OptScheme& scheme, uint leftIndex, uint rightIndex);

    bool isSwapResultsPairSuiteOptimizationTactics(SelectionFunc selectionFunc,
        const SwapResultsPair& result, uint leftIndex, uint rightIndex,
        uint* newLeftIndex, uint* newRightIndex);

    uint getMaximumTransferIndex(const OptScheme& scheme, const ReverseElement& target,
        uint startIndex, uint stopIndex) const;

    void moveElementInScheme(OptScheme* scheme, uint fromIndex, uint toIndex);

    // this flag is needed on the last optimization step
    // when none of the negative control inputs are allowed
    bool isNegativeControlInputsAllowed = true;
};

struct PostProcessor::OptimizationParams
{
    OptimizationParams() = default;

    bool inversions = false;
    bool heavyRight = false;
    // Remove flag suppress "as is" flag
    bool remove     = false;
    bool replace    = false;
    list<ReverseElement> replacement;
    bool asis       = true;
};

}   // ReversibleLogic