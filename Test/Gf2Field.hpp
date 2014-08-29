#pragma once

class Gf2Field
{
public:
    Gf2Field() = default;
    Gf2Field(word base);

    uint getDegree() const;

    bool has(word x) const;

    word add(word x, word y) const;
    word mul(word x, word y) const;
    word pow(word x, word n) const;

    // Returns wordUndefined if none was found
    word getPrimitiveElement();

private:
    uint getPolynomDegree(word x) const;

    word base   = 0;
    uint degree = 0;
    
    word primitiveElement = wordUndefined;
};
