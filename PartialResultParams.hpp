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

struct PartialResultParams
{
    PartialResultParams();
    virtual ~PartialResultParams() = default;

    bool isBetterThan( const PartialResultParams& another ) const;
    bool operator<( const PartialResultParams& another ) const;

    word getCoveredTranspositionsCount() const;

    enum PartialResultType
    {
        tNone = 0,
        tCommonPair,
        tSameDiffPair,
        tPack,
        tEdge,
        tFullEdge,
    };

    PartialResultType type;
    uint distancesSum;

    shared_ptr<list<ReversibleLogic::Transposition>> transpositions;
    BooleanEdge edge;

    struct
    {
        word edgeCapacity;  // tFullEdge and tEdge
        word packSize;      // tPack
        word diff;          // tSameDiffPair

        struct              // tCommonPair
        {
            word leftDiff;
            word rightDiff;
            word distance;
        } common;

    } params;
};
