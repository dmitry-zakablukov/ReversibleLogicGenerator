#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    Scheme generate(const TruthTable& inputTable, ostream& outputLog);

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
};

} //namespace ReversibleLogic