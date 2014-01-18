#pragma once

enum PartialResultType
{
    tNone = 0,
    tCommonPair,
    tSameDiffPair,
    tEdge,
    tFullEdge,
};

struct PartialResultParams
{
    PartialResultType type;
    uint restCyclesDistanceSum;

    union
    {
        word edgeCapacity;  // tFullEdge and tEdge
        word diff;          // tSameDiffPair

        struct              // tCommonPair
        {
            word leftDiff;
            word rightDiff;
            word distance;
        } common;

    } params;
};
