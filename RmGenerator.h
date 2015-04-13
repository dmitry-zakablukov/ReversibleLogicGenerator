#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    /// @threshold - spectra rows with indices, which weight greater than this value,
    /// would not be processed
    explicit RmGenerator(uint threshold = uintUndefined);
    virtual ~RmGenerator() = default;

    struct SynthesisResult
    {
        Scheme scheme; //synthesized scheme
        TruthTable residualTable;
    };

    void generate(const TruthTable& inputTable, SynthesisResult* result);

private:
    TruthTable invertTable(const TruthTable& directTable) const;

    struct SynthesisParams
    {
        TruthTable table;

        RmSpectra spectra;        
        uint spectraCost;

        deque<ReverseElement> elements;
    };

    void calculatePartialResult(SynthesisParams* params, uint n, uint index);

    struct PushedTranspositions
    {
        deque<Transposition> left;
        deque<Transposition> right;
    };

    /// Processes spectra row with index which weight is more than threshold
    void processAlienSpectraRow(uint n, uint index, const Scheme& scheme,
        Scheme::const_iterator iter, PushedTranspositions* transpositions);

    template<typename Iterator>
    word conjugateValue(word x, Iterator from, Iterator end) const ;

    void processFirstSpectraRow(SynthesisParams* params, uint n);
    void processVariableSpectraRow(SynthesisParams* params, uint n, uint index);
    void processNonVariableSpectraRow(SynthesisParams* params, uint n, uint index);

    template<typename TableType>
    void applyTransformation(TableType* tablePtr, word targetMask, word controlMask = 0);

    bool isInverseParamsBetter() const;

    template<typename Iterator>
    Scheme::iterator updateScheme(Scheme* scheme, Scheme::iterator iter,
        Iterator from, Iterator to);

    TruthTable getResidualTruthTable(const PushedTranspositions& transpositions, uint size) const;

    SynthesisParams directParams;
    SynthesisParams inverseParams;

    uint weightThreshold;
};

} //namespace ReversibleLogic