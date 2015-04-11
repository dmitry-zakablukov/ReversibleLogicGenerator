#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    Scheme generate(const TruthTable& inputTable, ostream& outputLog);

private:
    TruthTable invertTable(const TruthTable& directTable) const;

    void processFirstSpectraRow(Scheme* scheme, TruthTable* table, RmSpectra* spectra, uint n);

    void processVariableSpectraRow(Scheme* scheme, TruthTable* table, RmSpectra* spectra,
        uint n, uint index);

    void processNonVariableSpectraRow(Scheme* scheme, TruthTable* tablePtr, RmSpectra* spectraPtr,
        uint n, uint index);

    template<typename TableType>
    void applyTransformation(TableType* tablePtr, word targetMask, word controlMask = 0);

    struct SynthesisParams
    {
        TruthTable table;

        RmSpectra spectra;        
        uint spectraCost;

        deque<ReverseElement> elements;
    };

    SynthesisParams directParams;
    SynthesisParams inverseParams;
};

} //namespace ReversibleLogic