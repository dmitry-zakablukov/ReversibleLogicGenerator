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

class Permutation
{
public:
    Permutation();
    explicit Permutation(vector<shared_ptr<Cycle>> theCycles);
    virtual ~Permutation() = default;

    void append(shared_ptr<Cycle> cycle);

    uint length() const;
    shared_ptr<Cycle> getCycle(uint index) const;

    uint getElementCount() const;
    uint getTranspositionsCount() const;

    bool isEmpty() const;

    bool isEven() const;

    void completeToEven();
    void completeToEven(word truthTableSize);

    friend ostream& operator <<(ostream& out, const Permutation& permutation);

    /// For range-based for loop
    vector<shared_ptr<Cycle>>::const_iterator begin() const;
    vector<shared_ptr<Cycle>>::const_iterator end() const;

    Permutation multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication) const;

    Permutation multiplyByTranspositions(const list<Transposition>& transpositions,
        bool isLeftMultiplication) const;

    uint getDistancesSum() const;

    Permutation clone() const;

private:
    vector<shared_ptr<Cycle>> cycles;
};

}   // namespace ReversibleLogic
