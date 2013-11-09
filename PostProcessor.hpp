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

    Scheme mergeOptimization(Scheme& scheme, bool* optimized = 0 );

    Scheme getFullScheme(const Scheme& scheme, bool heavyRight = true);

    // Remove duplicates elements
    Scheme removeDuplicates(const Scheme& scheme);

    Scheme getFinalSchemeImplementation(const Scheme& scheme);

    struct Optimizations;
    vector<Optimizations> optimizations;
};

struct PostProcessor::Optimizations
{
    Optimizations();

    bool inversions;
    bool heavyRight;
    // Remove flag suppress "as is" flag
    bool remove;
    bool asis;
};

}   // ReversibleLogic