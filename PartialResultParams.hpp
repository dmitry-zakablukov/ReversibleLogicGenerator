#pragma once

struct PartialResultParams
{
    PartialResultParams();
    virtual ~PartialResultParams() = default;

    bool isBetterThan( const PartialResultParams& another ) const;
    bool operator<( const PartialResultParams& another ) const;

    word getCoveredTranspositionsCount() const;

    enum PartialResultType
    {
        tNone = 0,
        tCommonPair,
        tSameDiffPair,
        tPack,
        tEdge,
        tFullEdge,
    };

    PartialResultType type;
    uint distancesSum;

    shared_ptr<list<ReversibleLogic::Transposition>> transpositions;
    BooleanEdge edge;

    struct
    {
        word edgeCapacity;  // tFullEdge and tEdge
        word packSize;      // tPack
        word diff;          // tSameDiffPair

        struct              // tCommonPair
        {
            word leftDiff;
            word rightDiff;
            word distance;
        } common;

    } params;
};
