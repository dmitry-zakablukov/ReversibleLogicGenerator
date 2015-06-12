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

Transposition::Transposition(word left, word right, bool needSort /* = false */)
    : empty(false)
{
    if(left == right)
    {
        ostringstream stream;
        stream << "Transposition constructor failed with x = ";
        stream << left << " and y = " << right;

        assertd(false, stream.str());
    }

    x = left;
    y = right;

    if(needSort)
        sort();
}

Transposition::Transposition(const Transposition& another)
    : x(another.x)
    , y(another.y)
    , empty(another.empty)
{
}

bool Transposition::isEmpty() const
{
    return empty;
}

void Transposition::sort()
{
    uint xWeight = countNonZeroBits(x);
    uint yWeight = countNonZeroBits(y);

    if(xWeight > yWeight)
        swap();
}

void Transposition::swap()
{
    std::swap(x, y);
}

void Transposition::setX(word value)
{
    x = value;
    empty = false;
}

word Transposition::getX() const
{
    return x;
}

void Transposition::setY(word value)
{
    y = value;
    empty = false;
}

word Transposition::getY() const
{
    return y;
}

word Transposition::getDiff() const
{
    return x ^ y;
}

uint Transposition::getDist() const
{
    return countNonZeroBits(x ^ y);
}

bool Transposition::has(word value) const
{
    bool result = (x == value) || (y == value);
    return result;
}

bool Transposition::operator ==(const Transposition& another) const
{
    word z = another.getX();
    word w = another.getY();

    bool isEqual = ((x == z && y == w) || (x == w && y == z));
    return isEqual;
}

bool Transposition::operator !=(const Transposition& another) const
{
    return !operator ==(another);
}

ostream& operator <<(ostream& out, const Transposition& t)
{
    out << "(" << t.x << ", " << t.y << ")";
    return out;
}

word Transposition::getOutput(word input) const
{
    word output = input;
    if(output == x)
        output = y;
    else if (output == y)
        output = x;

    return output;
}

}   // namespace ReversibleLogic
