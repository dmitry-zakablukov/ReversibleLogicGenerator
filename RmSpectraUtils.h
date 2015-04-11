#pragma once

namespace ReversibleLogic
{

typedef vector<word> RmSpectra;

class RmSpectraUtils
{
public:
    /// Returns Reed-Muller spectra for input truth @table
    static RmSpectra calculateSpectra(const TruthTable& table);

    static bool isVariableRow(uint index);

    /// Returns row of RM-spectra of identity function by @index
    static word getRowOfIdentSpectra(uint index);

    /// Returns true if @row in spectra equals to row with the same index in RM-spectra of identity function
    static bool isSpectraRowIdent(const RmSpectra& spectra, uint index);
    static bool isSpectraRowIdent(word row, uint index);

    /// Returns cost of Reed-Muller spectra
    static uint calculateCost(const RmSpectra& spectra);
};

} //namespace ReversibleLogic