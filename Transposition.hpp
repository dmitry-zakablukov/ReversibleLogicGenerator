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

class Transposition
{
public:
    Transposition() = default;
    explicit Transposition(word left, word right, bool needSort = false);
    Transposition(const Transposition& another);

    virtual ~Transposition() = default;

    bool isEmpty() const;

    /// Sort x and y by they weights
    void sort();

    /// Swap x and y
    void swap();

    void setX(word value);
    word getX() const;

    void setY(word value);
    word getY() const;

    /// Returns difference between x and y
    word getDiff() const;

    /// Returns distance between x and y
    uint getDist() const;

    /// Returns true if transposition has specified value
    bool has(word value) const;

    /// Returns new value for input if it is in transposition
    /// i.e. input == x, then output = y and vice versa
    word getOutput(word input) const;

    bool operator ==(const Transposition& another) const;
    bool operator !=(const Transposition& another) const;

    friend ostream& operator <<(ostream& out, const Transposition& t);

private:
    word x = 0;
    word y = 0;

    bool empty = true;
};

}   // namespace ReversibleLogic
