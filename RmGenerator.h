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
        Scheme::iterator iter; //position in scheme, where residual sub-scheme should be inserted

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

    void processFirstSpectraRow(SynthesisParams* params, uint n);
    void processVariableSpectraRow(SynthesisParams* params, uint n, uint index);
    void processNonVariableSpectraRow(SynthesisParams* params, uint n, uint index);

    template<typename TableType>
    void applyTransformation(TableType* tablePtr, word targetMask, word controlMask = 0);

    bool isInverseParamsBetter() const;

    template<typename IteratorType>
    Scheme::iterator updateScheme(Scheme* scheme, Scheme::iterator iter,
        IteratorType from, IteratorType to);

    SynthesisParams directParams;
    SynthesisParams inverseParams;

    uint weightThreshold;
};

} //namespace ReversibleLogic