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

#include "std.hpp"

word mask(uint first, ...)
{
    word result = (word)1 << first;

    va_list list;
    va_start(list, first);
    uint temp = va_arg(list, uint);

    while(temp != uintUndefined)
    {
        result += (word)1 << temp;
        temp = va_arg(list, uint);
    }

    return result;
}

word binStringToInt(string value)
{
    word number = 0;
    for (auto& symbol : value)
    {
        number <<= 1;
        if(symbol == '1')
            number |= 1;
    }

    return number;
}

string polynomialToString(word polynomial)
{
    ostringstream repres;

    uint index = 0;
    word mask = 1;

    while (mask <= polynomial)
    {
        if (polynomial & mask)
        {
            if (index)
            {
                if (polynomial & 1)
                    repres << " +";

                if (index == 1)
                    repres << " x";
                else
                    repres << " " << index << "^x";
            }
            else
                repres << "1";
        }

        mask <<= 1;
        ++index;
    }

    string result = repres.str();
    reverse(result.begin(), result.end());
    return result;
}

vector<word> makePermutationFromScheme(ReversibleLogic::Scheme scheme,
    uint n)
{
    using namespace ReversibleLogic;

    word transformCount = 1 << n;
    vector<word> table(transformCount);

    for(word x = 0; x < transformCount; ++x)
    {
        word y = x;
        for(Scheme::const_iterator iter = scheme.begin();
            iter != scheme.end(); ++iter)
        {
            y = (*iter).getValue(y);
            assertd((y < transformCount), string("Wrong transform"));
        }

        table[x] = y;
    }

    return table;
}
