#include "std.hpp"

namespace ReversibleLogic
{

//static
RmSpectra RmSpectraUtils::calculateSpectra(const TruthTable& table)
{
    uint size = table.size();
    assertd(countNonZeroBits(size) == 1,
        string("RmSpectraUtils::calculateRmSpectra(): truth table size should be power of two"));

    RmSpectra spectra = table;
    auto spectraPtr = spectra.data();

    for (uint step = 1; step < size; step <<= 1)
    {
        for (uint index = 0; index < size; index += 2 * step)
        {
            auto abovePtr = spectraPtr + index;
            auto belowPtr = abovePtr + step;

            uint count = step;
            while (count--)
            {
                *belowPtr ^= *abovePtr;

                ++abovePtr;
                ++belowPtr;
            }
        }
    }

    return spectra;
}

//static
bool RmSpectraUtils::isVariableRow(uint index)
{
    return countNonZeroBits(index) == 1;
}

//static
word RmSpectraUtils::getRowOfIdentSpectra(uint index)
{
    word row = 0;
    if (isVariableRow(index))
        row = (word)index;

    return row;
}

//static
bool RmSpectraUtils::isSpectraRowIdent(const RmSpectra& spectra, uint index)
{
    return isSpectraRowIdent(spectra[index], index);
}

//static
bool RmSpectraUtils::isSpectraRowIdent(word row, uint index)
{
    return row == getRowOfIdentSpectra(index);
}

uint RmSpectraUtils::calculateCost(const RmSpectra& spectra)
{
    uint cost = 0;
    uint size = spectra.size();

    for (uint index = 0; index < size; ++index)
    {
        word indentRow = getRowOfIdentSpectra(index);
        word row = spectra[index];

        cost += countNonZeroBits(row ^ indentRow);
    }

    return cost;
}

} //namespace ReversibleLogic