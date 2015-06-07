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

Gf2Field::Gf2Field(word base)
    : base(base)
{
    degree = getPolynomDegree(base);
    assertd(degree > 0, string("Base have no positive degree"));
}

uint Gf2Field::getDegree() const
{
    return degree;
}

bool Gf2Field::has(word x) const
{
    bool result = (x > 0 && x < (word)(1 << degree));
    return result;
}

word Gf2Field::add(word x, word y) const
{
    word sum = x + y;
    word baseMask = 1 << degree;

    // max additional result is double as x, so check only one bit
    if (sum & baseMask)
        sum ^= base;

    return sum;
}

word Gf2Field::mul(word x, word y) const
{
    assertd(has(x), string("x not in field"));
    assertd(has(y), string("y not in field"));

    uint yDegree = getPolynomDegree(y);
    word yMask = 1 << yDegree;

    word baseMask = 1 << degree;

    word result = x;
    yMask >>= 1;

    while(yMask)
    {
        result <<= 1;
        if(y & yMask)
        {
            result ^= x;
        }

        yMask >>= 1;
        if(result & baseMask)
        {
            result ^= base;
        }
    }

    return result;
}

word Gf2Field::pow(word x, word n) const
{
    assertd(has(x), string("x not in field"));
    assertd(n >= 0, string("Degree is non positive"));

    uint nDegree = getPolynomDegree(n);
    word nMask = 1 << nDegree;

    word result = 1;
    while(nMask)
    {
        result = mul(result, result);
        if(n & nMask)
        {
            result = mul(result, x);
        }

        nMask >>= 1;
    }

    return result;
}

uint Gf2Field::getPolynomDegree(word x) const
{
    uint degree = 0;
    while(x > 1)
    {
        ++degree;
        x >>= 1;
    }

    return degree;
}

word Gf2Field::getPrimitiveElement()
{
    if (primitiveElement != wordUndefined)
        return primitiveElement;

    word count = 1 << getDegree();
    if (count == 2)
    {
        primitiveElement = 1;
        return primitiveElement;
    }

    for (word element = 2; element < count; ++element)
    {
        word z = element;
        word attempts = count - 3; //0, 1, count degrees can be skipped

        while (attempts--)
        {
            z = mul(z, element);
            if (z == 1)
            {
                z = wordUndefined;
                break;
            }
        }

        if (z != wordUndefined)
        {
            primitiveElement = element;
            break;
        }
    }

    return primitiveElement;
}
