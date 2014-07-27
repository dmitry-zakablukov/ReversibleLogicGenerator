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

    // First element - new reverse element obtained as a result of swapping
    // Second element - range of indices, on which this element is freely swappable
    typedef pair<ReverseElement, Range> SwapResult;

    // Returns swap result for element on @startIndex position in @scheme.
    // Element on @skipIndex position doesn't participate in this operation.
    // Param @toLeft determines, where this element is moving.
    deque<SwapResult> getSwapResult(const OptScheme& scheme, uint startIndex,
        uint skipIndex, bool toLeft = true);

    deque<SwapResult> mergeSwapResults(deque<SwapResult> toLeft, deque<SwapResult> toRight);

    // First element of pair - swap results for left element
    // Second element of pair - swap results for right element
    typedef struct 
    {
        deque<SwapResult> forLeft;
        deque<SwapResult> forRight;
    } SwapResultsPair;
        
    void getSwapResultsPair(SwapResultsPair* result, const OptScheme& scheme,
        uint leftIndex, uint rightIndex);

    bool isSwapResultsPairSuiteOptimizationTactics(SelectionFunc selectionFunc,
        const SwapResultsPair& result, uint* newLeftIndex, uint* newRightIndex);

    int getMaximumTransferIndex(const OptScheme& scheme, const ReverseElement& target,
        int startIndex, int stopIndex) const;

    /// Inserts replacements to result scheme based on original scheme
    void insertReplacements(const OptScheme& originalScheme,
        OptScheme* resultScheme,
        int leftIndex, int leftTransferedIndex,
        int rightIndex, int rightTransferedIndex,
        const list<ReverseElement>& leftReplacement,
        const list<ReverseElement>& rightReplacement);

    // this flag turns on and off second transfer optimization
    // inside transfer optimization, if less scheme complexity
    // is required
    bool secondPassOptimizationFlag;

    OptScheme testScheme;
    int complexityDelta;
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