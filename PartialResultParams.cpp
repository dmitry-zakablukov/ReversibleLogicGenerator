#include "std.hpp"


PartialResultParams::PartialResultParams()
    : type(tNone)
    , distancesSum(0)
    , transpositions(0)
    , edge(uintUndefined)
{
    memset(&params, 0, sizeof(params));
}

bool PartialResultParams::isBetterThan(const PartialResultParams& another) const
{
    bool isLeftBetter = false;

#define IS_LEFT_BETTER(left, right) ((left) <= (right))

    if(type == another.type)
    {
        switch(type)
        {
        case tFullEdge:
        case tEdge:
            if(params.edgeCapacity == another.params.edgeCapacity)
                isLeftBetter = IS_LEFT_BETTER(distancesSum, another.distancesSum);
            else
                isLeftBetter = (params.edgeCapacity > another.params.edgeCapacity);
            break;

        case tSameDiffPair:
            {
                uint leftWeight  = countNonZeroBits(params.diff);
                uint rightWeight = countNonZeroBits(another.params.diff);

                if(leftWeight == rightWeight)
                    isLeftBetter = IS_LEFT_BETTER(distancesSum, another.distancesSum);
                else
                    isLeftBetter = (leftWeight < rightWeight);
            }
            break;

        case tPack:
            isLeftBetter = params.packSize >= another.params.packSize;
            break;

        case tCommonPair:
            {
                uint leftSum = 0;

                leftSum += countNonZeroBits(params.common.leftDiff );
                leftSum += countNonZeroBits(params.common.rightDiff);
                leftSum += countNonZeroBits(params.common.distance );

                uint rightSum = 0;

                rightSum += countNonZeroBits(another.params.common.leftDiff );
                rightSum += countNonZeroBits(another.params.common.rightDiff);
                rightSum += countNonZeroBits(another.params.common.distance );

                if(leftSum == rightSum)
                    isLeftBetter = IS_LEFT_BETTER(distancesSum, another.distancesSum);
                else
                    isLeftBetter = (leftSum < rightSum);
            }
            break;

        default:
            assert(false, string("PartialResultParams: can't compare with unknown partial result type"));
            break;
        }
    }
    else
    {
        isLeftBetter = type > another.type;
    }

#undef IS_LEFT_BETTER

    return isLeftBetter;
}

bool PartialResultParams::operator<(const PartialResultParams& another) const
{
    return isBetterThan(another);
}

word PartialResultParams::getCoveredTranspositionsCount() const
{
    word count = 0;
    switch(type)
    {
    case tFullEdge:
    case tEdge:
        count = params.edgeCapacity / 2;
        break;

    case tPack:
        count = params.packSize;
        break;

    case tSameDiffPair:
    case tCommonPair:
        count = 2;
        break;
    }

    return count;
}
