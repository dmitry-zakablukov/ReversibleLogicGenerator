// Group theory based generator class for reversible logic synthesis with additional input lines

#pragma once

namespace ReversibleLogic
{

class GtGeneratorWithMemory
{
public:
    GtGeneratorWithMemory() = default;
    virtual ~GtGeneratorWithMemory() = default;

    Scheme generateFast(const TruthTable& table, ostream& outputLog);

private:
    void detectBitCount(const TruthTable& table, uint* n, uint* m);

    void generateCoordinateFunction(Scheme* scheme,
        uint n, uint m, uint coord, unordered_set<word>& inputs);

    bool checkSchemeValidity(const Scheme& scheme, uint n, uint m,
        const TruthTable& table);
};

} //ReversibleLogic