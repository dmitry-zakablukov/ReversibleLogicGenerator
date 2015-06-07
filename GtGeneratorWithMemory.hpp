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

// Group theory based generator class for reversible logic synthesis with additional input lines

#pragma once

namespace ReversibleLogic
{

class GtGeneratorWithMemory
{
public:
    GtGeneratorWithMemory() = default;
    virtual ~GtGeneratorWithMemory() = default;

    Scheme generateFast(const TruthTable& table, ostream& outputLog);

private:
    void detectBitCount(const TruthTable& table, uint* n, uint* m);

    void generateCoordinateFunction(Scheme* scheme,
        uint n, uint m, uint coord, unordered_set<word>& inputs);

    bool checkSchemeValidity(const Scheme& scheme, uint n, uint m,
        const TruthTable& table);
};

} //ReversibleLogic