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

class GtGenerator
{
public:
    GtGenerator() = default;
    virtual ~GtGenerator() = default;

    Scheme generate(const TruthTable& table);

private:
    tuple<uint, Permutation> getPermutation(const TruthTable& table);
    void checkPermutationValidity(const TruthTable& table);

    void implementPartialResult(PartialGtGenerator& partialGenerator,
        bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter);

    shared_ptr<PartialGtGenerator> reducePermutation(shared_ptr<PartialGtGenerator> partialGenerator,
        uint n, Scheme* scheme, Scheme::iterator* targetIter);

    uint n = 0;
    Permutation permutation;
};

}   // namespace ReversibleLogic
