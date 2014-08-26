#include "std.hpp"

Gf2Field::Gf2Field()
    : base(0)
    , degree(0)
{

}

Gf2Field::Gf2Field(word base)
    : base(base)
{
    degree = getPolynomDegree(base);
    assert(degree > 0, string("Base have no positive degree"));
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

word Gf2Field::mul(word x, word y) const
{
    assert(has(x), string("x not in field"));
    assert(has(y), string("y not in field"));

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
    assert(has(x), string("x not in field"));
    assert(n >= 0, string("Degree is non positive"));

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
