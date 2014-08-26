#pragma once

class Gf2Field
{
public:
    Gf2Field();
    Gf2Field(word base);

    uint getDegree() const;

    bool has(word x) const;

    word mul(word x, word y) const;
    word pow(word x, word n) const;

private:
    uint getPolynomDegree(word x) const;

    word base;
    uint degree;
};
