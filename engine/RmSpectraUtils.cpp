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

#include "std.h"

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