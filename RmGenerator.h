#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    Scheme generate(const TruthTable& inputTable, ostream& outputLog);

private:
    void processFirstSpectraRow(Scheme* scheme, TruthTable* table, RmSpectra* spectra, uint n);

    void processVariableSpectraRow(Scheme* scheme, TruthTable* table, RmSpectra* spectra,
        uint n, uint index);

    void processNonVariableSpectraRow(Scheme* scheme, TruthTable* tablePtr, RmSpectra* spectraPtr,
        uint n, uint index);

    template<typename TableType>
    void applyTransformation(TableType* tablePtr, word targetMask, word controlMask = 0);
};

template<typename TableType>
void RmGenerator::applyTransformation(TableType* tablePtr, word targetMask, word controlMask /*= 0*/)
{
    assertd(tablePtr, string("RmGenerator::applyTransformation(): null ptr"));

    assertd(countNonZeroBits(targetMask) == 1 && (controlMask & targetMask) == 0,
        string("RmGenerator::applyTransformation(): invalid arguments"));

    TruthTable& table = *tablePtr;
    uint size = table.size();

    for (uint index = 0; index < size; ++index)
    {
        word value = table[index];
        if ((value & controlMask) == controlMask)
            table[index] = value ^ targetMask;
    }
}

} //namespace ReversibleLogic