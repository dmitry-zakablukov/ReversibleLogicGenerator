#pragma once

enum PartialResultType
{
    tFullEdge = 0,
    tEdge,
    tSameDiffPair,
    tCommonPair,
};

struct PartialResultParams
{
    PartialResultType type;

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
