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

class Cycle
{
public:
    Cycle();
    Cycle(vector<word>&& source);

    virtual ~Cycle() = default;

    /// Finalize cycle: none element could be added to cycle
    void finalize();

    /// Return true if cycle is finalized
    bool isFinal() const;

    /// Add new element to cycle (before finalization)
    void append(word element);

    uint length() const;
    bool isEmpty() const;

    const word& operator[](uint index) const;

    bool operator ==(const Cycle& another) const;
    bool has(const Transposition& target) const;
    bool has(word x) const;

    friend ostream& operator <<(ostream& out, const Cycle& cycle);

    /// Returns new value for input if it is in cycle
    /// i.e. output = elements[i+1] if input == elements[i]
    word getOutput(word input) const;

    /// @frequencyMap - dictionary for calculation diff frequency throw all cycles
    void prepareForDisjoint(unordered_map<word, uint>* frequencyMap);

    /// Finds all possible transpositions from cycle which have specified Hamming distance
    /// @diff - Hamming distance which should have all returned transpositions
    /// @result - output list for adding found transpositions
    void disjointByDiff(word diff, shared_ptr<list<Transposition>> result) const;

    void multiplyByTranspositions(shared_ptr<list<Transposition>> transpositions,
        bool isLeftMultiplication, vector<shared_ptr<Cycle>>* output) const;

    uint getDistancesSum() const;

private:
    /// Returns index modulo element count
    uint modIndex(uint index) const;
    uint modIndex(uint index, uint mod) const;

    /// Returns f_n(f_{n-1}(...(f_1(input))...)), where f_i is defined by i-th transposition in @transpositions
    word getOutput(word input, shared_ptr<list<Transposition>> transpositions) const;

    /// Gets all possible transpositions with specified Hamming distance @diff
    /// from vector of elements @input and puts them to @result
    void getTranspositionsByDiff(const vector<word>& input, word diff,
        shared_ptr<list<Transposition>> result) const;

    void getTranspositionsByDiff(const vector<word>& input, word diff,
        uint xIndex, uint yIndex, shared_ptr<list<Transposition>> result) const;

    vector<word> elements;
    bool finalized;
};

}   // namespace ReversibleLogic
