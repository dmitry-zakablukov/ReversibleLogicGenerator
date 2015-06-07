// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

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