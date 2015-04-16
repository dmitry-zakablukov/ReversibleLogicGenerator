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
        TruthTable leftMultTable;
        TruthTable rightMultTable;
    };

    void generate(const TruthTable& inputTable, SynthesisResult* result);

    deque<ReversibleLogic::ReverseElement>::iterator implementPartialResult(Scheme* scheme,
        deque<ReversibleLogic::ReverseElement>::iterator iter);

    struct PushPolicy
    {
        bool defaultPolicy = true;
        bool forceLeft = false;
        bool forceRight = false;
        bool autoHammingDistance = false;
        bool autoRmCostReduction = false;
    };

    const PushPolicy& getPushPolicy() const;

private:
    TruthTable invertTable(const TruthTable& directTable) const;

    void initResult(SynthesisResult* result, uint size);
    void initPushPolicy();
    void initSynthesisParams(const TruthTable& inputTable);

    struct SynthesisParams
    {
        TruthTable table;

        RmSpectra spectra;        
        uint spectraCost;

        deque<ReverseElement> elements;
    };

    void calculatePartialResult(SynthesisParams* params, uint n, uint index);

    /// Processes spectra row with index which weight is more than threshold
    void processAlienSpectraRow(uint n, uint index, const Scheme& scheme,
        Scheme::const_iterator iter, SynthesisResult* result);

    template<typename Iterator>
    word conjugateValue(word x, Iterator from, Iterator end) const;

    void pushTranpsositionToLeft(const Transposition& transp, SynthesisResult* result);
    void pushTranpsositionToRight(const Transposition& transp, SynthesisResult* result);

    void processFirstSpectraRow(SynthesisParams* params, uint n);
    void processVariableSpectraRow(SynthesisParams* params, uint n, uint index);
    void processNonVariableSpectraRow(SynthesisParams* params, uint n, uint index);

    template<typename TableType>
    void applyTransformation(TableType* tablePtr, word targetMask, word controlMask = 0);

    bool isInverseParamsBetter() const;

    template<typename Iterator>
    Scheme::iterator updateScheme(Scheme* scheme, Scheme::iterator iter,
        Iterator from, Iterator to);

    SynthesisParams directParams;
    SynthesisParams inverseParams;

    PushPolicy pushPolicy;

    uint weightThreshold;
};

} //namespace ReversibleLogic