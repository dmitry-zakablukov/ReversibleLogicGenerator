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

#include "std.h"

namespace ReversibleLogic
{

//////////////////////////////////////////////////////////////////////////
// Selection functions
//////////////////////////////////////////////////////////////////////////
bool selectEqual(const ReverseElement& left, const ReverseElement& right)
{
    return left == right;
}

bool selectForReduceConnectionsOptimization(const ReverseElement& left, const ReverseElement& right)
{
    // (01)(10) -> (*1)(1*)

    bool result = false;
    if(left.getTargetMask() == right.getTargetMask()
        && left.getControlMask() == right.getControlMask())
    {
        word  leftInversionMask =  left.getInversionMask();
        word rightInversionMask = right.getInversionMask();

        word diffInversionMask = leftInversionMask ^ rightInversionMask;
        word controlValue = leftInversionMask & diffInversionMask;

        result = (countNonZeroBits(diffInversionMask) == 2
            && countNonZeroBits(controlValue) == 1);
    }

    return result;
}

bool selectForMergeOptimization(const ReverseElement& left, const ReverseElement& right)
{
    // (01)(11) -> (*1)
    // (0*)(01) -> (00)
    // (1*)(10) -> (11)

    if(left.getTargetMask() == right.getTargetMask())
    {
        word  leftControlMask =  left.getControlMask();
        word rightControlMask = right.getControlMask();

        word  leftInversionMask =  left.getInversionMask();
        word rightInversionMask = right.getInversionMask();

        word controlsDiff   = leftControlMask   ^ rightControlMask;
        word inversionsDiff = leftInversionMask ^ rightInversionMask;

        // (01)(11) -> (*1)
        if (leftControlMask == rightControlMask && countNonZeroBits(inversionsDiff) == 1)
            return true;
 
        // (0*)(01) -> (00)
        if (leftInversionMask == rightInversionMask && countNonZeroBits(controlsDiff) == 1)
            return true;

        // (1*)(10) -> (11)
        if (controlsDiff == inversionsDiff && countNonZeroBits(controlsDiff) == 1)
            return true;
    }

    return false;
}

bool selectForMergeOptimizationWithoutInversions(const ReverseElement& left, const ReverseElement& right)
{
    // (01)(11) -> (*1)
    // (1*)(10) -> (11)

    if (left.getTargetMask() == right.getTargetMask())
    {
        word  leftControlMask = left.getControlMask();
        word rightControlMask = right.getControlMask();

        word  leftInversionMask = left.getInversionMask();
        word rightInversionMask = right.getInversionMask();

        word controlsDiff = leftControlMask   ^ rightControlMask;
        word inversionsDiff = leftInversionMask ^ rightInversionMask;

        // (01)(11) -> (*1)
        if (leftControlMask == rightControlMask && countNonZeroBits(inversionsDiff) == 1)
            return true;

        // (1*)(10) -> (11)
        if (controlsDiff == inversionsDiff && countNonZeroBits(controlsDiff) == 1)
            return true;
    }

    return false;
}

bool selectForTransferOptimization(const ReverseElement& left, const ReverseElement& right)
{
    word leftTargetMask   =  left.getTargetMask();
    word leftControlMask  =  left.getControlMask();

    word rightTargetMask  = right.getTargetMask();
    word rightControlMask = right.getControlMask();

    bool result = false;
    bool withOneControlLineInverting = false;

    if (!left.isSwappable(right, &withOneControlLineInverting) || withOneControlLineInverting)
    {
        // (left_target in right_controls) xor (right_target in left_controls)
        result = ((rightControlMask & leftTargetMask) != 0) ^ ((leftControlMask & rightTargetMask) != 0);
    }

    return result;
}

bool selectForPeresGateOptimization(const ReverseElement& left, const ReverseElement& right)
{
    word leftTargetMask = left.getTargetMask();
    word leftControlMask = left.getControlMask();

    word rightTargetMask = right.getTargetMask();
    word rightControlMask = right.getControlMask();

    uint leftCount = countNonZeroBits(leftControlMask);
    uint rightCount = countNonZeroBits(rightControlMask);

    return (
        (leftCount == 2 && rightCount == 1 && leftControlMask == (rightTargetMask | rightControlMask)) ||
        (rightCount == 2 && leftCount == 1 && rightControlMask == (leftTargetMask | leftControlMask)));
}

//////////////////////////////////////////////////////////////////////////
// Swap functions
//////////////////////////////////////////////////////////////////////////
void swapEqualElements(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assertd(selectEqual(left, right), string("swapEqualElements(): wrong input elements"));

    // leave replacements empty
    leftReplacement->resize(0);
    rightReplacement->resize(0);
}

void swapElementsWithConnectionReduction(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assertd(selectForReduceConnectionsOptimization(left, right),
        string("swapElementsWithConnectionReduction(): wrong input elements"));

    word  leftInversionMask =  left.getInversionMask();
    word rightInversionMask = right.getInversionMask();
    word diffInversionMask  = leftInversionMask ^ rightInversionMask;

    // important! fill only left replacement
    ReverseElement newLeft = left;
    word clearMask = diffInversionMask & leftInversionMask;

    newLeft.setControlMask(left.getControlMask() & ~clearMask);
    newLeft.setInversionMask(leftInversionMask & ~clearMask);

    leftReplacement->push_back(newLeft);

    ReverseElement newRight = right;
    clearMask = diffInversionMask & rightInversionMask;

    newRight.setControlMask(right.getControlMask() & ~clearMask);
    newRight.setInversionMask(rightInversionMask & ~clearMask);

    leftReplacement->push_back(newRight);
}

void swapElementsWithMerge(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    // (01)(11) -> (*1)
    // (0*)(01) -> (00)
    // (1*)(10) -> (11)

    if (left.getTargetMask() == right.getTargetMask())
    {
        word  leftControlMask = left.getControlMask();
        word rightControlMask = right.getControlMask();

        word  leftInversionMask = left.getInversionMask();
        word rightInversionMask = right.getInversionMask();

        word controlsDiff = leftControlMask   ^ rightControlMask;
        word inversionsDiff = leftInversionMask ^ rightInversionMask;

        // (01)(11) -> (*1)
        if (leftControlMask == rightControlMask &&
            countNonZeroBits(inversionsDiff) == 1)
        {
            ReverseElement element = left;
            element.setControlMask(leftControlMask & ~inversionsDiff);
            element.setInversionMask(leftInversionMask & ~inversionsDiff);

            leftReplacement->push_back(element);
        }

        // (0*)(01) -> (00)
        if (leftInversionMask == rightInversionMask &&
            countNonZeroBits(controlsDiff) == 1)
        {
            ReverseElement element = left;
            element.setControlMask(leftControlMask | controlsDiff);
            element.setInversionMask(leftInversionMask | controlsDiff);

            leftReplacement->push_back(element);
        }

        // (1*)(10) -> (11)
        if (controlsDiff == inversionsDiff && countNonZeroBits(controlsDiff) == 1)
        {
            ReverseElement element = left;
            element.setControlMask(leftControlMask | controlsDiff);
            element.setInversionMask(leftInversionMask & ~inversionsDiff);

            leftReplacement->push_back(element);
        }
    }

    // important! only left replacement should be non-empty
    assertd(leftReplacement->size(), string("swapElementsWithMerge(): wrong input elements"));
}

void swapElementsWithTransferOptimization(const ReverseElement& leftElement, const ReverseElement& rightElement,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assertd(selectForTransferOptimization(leftElement, rightElement),
        string("swapElementsWithTransferOptimization(): wrong input elements"));

    const ReverseElement*  left =  &leftElement;
    const ReverseElement* right = &rightElement;

    // unify swap
    if(left->getControlMask() & right->getTargetMask())
    {
        const ReverseElement* tempElement = right;
        right = left;
        left  = tempElement;

        list<ReverseElement>* tempReplacement = rightReplacement;
        rightReplacement = leftReplacement;
        leftReplacement  = tempReplacement;
    }

    word leftTargetMask     =  left->getTargetMask();
    word leftControlMask    =  left->getControlMask();
    word leftInversionMask  =  left->getInversionMask();
                            
    word rightTargetMask    = right->getTargetMask();
    word rightControlMask   = right->getControlMask();
    word rightInversionMask = right->getInversionMask();

    // fill left replacement
    leftReplacement->push_back(*right);

    ReverseElement element = *right;
    word controlMask = ~leftTargetMask & (leftControlMask | rightControlMask);
    element.setControlMask(controlMask);

    word inversionMask = ~leftTargetMask & (leftInversionMask | rightInversionMask);
    element.setInversionMask(inversionMask);

    leftReplacement->push_back(element);

    // fill right replacement
    rightReplacement->push_back(*left);
}

void swapPeresElements(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assertd(selectForPeresGateOptimization(left, right), string("swapPeresElements(): wrong input elements"));

    // leave elements as is
    leftReplacement->push_back(left);
    rightReplacement->push_back(right);
}


//////////////////////////////////////////////////////////////////////////
ReversibleLogic::Scheme PostProcessor::optimize(const Scheme& scheme)
{
    uint elementCount = scheme.size();
    OptScheme optimizedScheme(elementCount);

    for (uint index = 0; index < elementCount; ++index)
        optimizedScheme[index] = scheme[index];

    optimizedScheme = optimize(optimizedScheme);
    elementCount = optimizedScheme.size();

    Scheme result(elementCount);
    for (uint index = 0; index < elementCount; ++index)
        result[index] = optimizedScheme[index];

    return result;
}

PostProcessor::OptScheme PostProcessor::optimize(const OptScheme& scheme)
{
    if (!ProgramOptions::get().doPostOptimization)
        return scheme;

    OptScheme optimizedScheme = scheme;
    
    uint lengthBefore = uintUndefined;
    uint lengthAfter  = uintUndefined;
    isNegativeControlInputsAllowed = true;

    bool needOptimization = true;
    while(needOptimization)
    {
        optimizedScheme = removeDuplicates(optimizedScheme);
        
        needOptimization = false;
        optimizedScheme = mergeOptimization(optimizedScheme, &needOptimization);
    }

    // final implementation
    OptScheme implementation;
    implementation = optimizedScheme;

    if (ProgramOptions::get().options.getBool("do-last-optimizations-with-full-scheme", true))
    {
        isNegativeControlInputsAllowed = true;

        implementation = getFullScheme(implementation, fstRecursive);
        implementation = removeDuplicates(implementation);

        needOptimization = true;
        while (needOptimization)
        {
            needOptimization = false;
            implementation = transferOptimization(implementation, &needOptimization);

            bool additional = false;
            implementation = mergeOptimization(implementation, &additional);
            needOptimization = needOptimization || additional;
        }

        isNegativeControlInputsAllowed = true;

        implementation = getFullScheme(implementation, fstSimple);
        implementation = removeDuplicates(implementation);

        needOptimization = true;
        while (needOptimization)
        {
            needOptimization = false;
            implementation = transferOptimization(implementation, &needOptimization);

            bool additional = false;
            implementation = mergeOptimization(implementation, &additional);
            needOptimization = needOptimization || additional;
        }

        if (ProgramOptions::get().options.getBool("remove-negative-control-inputs", false))
        {
            isNegativeControlInputsAllowed = false;

            implementation = getFullScheme(implementation, fstSimple);
            implementation = removeDuplicates(implementation);

            needOptimization = true;
            while (needOptimization)
            {
                needOptimization = false;
                implementation = transferOptimization(implementation, &needOptimization);

                bool additional = false;
                implementation = mergeOptimization(implementation, &additional);
                needOptimization = needOptimization || additional;
            }
        }
    }

    implementation = peresGateOptimization(implementation);

    return implementation;
}

void PostProcessor::prepareSchemeForOptimization(const OptScheme& scheme,
    Optimizations* optimizations)
{
    uint elementCount = scheme.size();
    optimizations->resize(elementCount);

    for(uint index = 0; index < elementCount; ++index)
        (*optimizations)[index] = OptimizationParams();
}

PostProcessor::OptScheme PostProcessor::applyOptimizations(const OptScheme& scheme,
    const Optimizations& optimizations)
{
    uint elementCount = scheme.size();
    OptScheme optimizedScheme;
    optimizedScheme.reserve(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        const OptimizationParams& optimization = optimizations[index];
        if(!optimization.remove)
        {
            // replace element by another elements
            if(optimization.replace)
            {
                optimizedScheme.insert(optimizedScheme.end(),
                    optimization.replacement.cbegin(),
                    optimization.replacement.cend());
            }
            // leave element as is
            else if(optimization.asis)
                optimizedScheme.insert(optimizedScheme.end(), scheme[index]);
            else
            {
                if(optimization.inversions) //inversions
                {
                    bool heavyRight = optimization.heavyRight;
                    auto impl = scheme[index].getInversionOptimizedImplementation(heavyRight);
                    optimizedScheme.insert(optimizedScheme.end(), impl.begin(), impl.end());
                }
                else //leave element as is
                    optimizedScheme.insert(optimizedScheme.end(), scheme[index]);
            }
        }
    }

    return optimizedScheme;
}

uint PostProcessor::findInversedElementsSequence(const OptScheme& scheme, uint startPosition)
{
    uint sequenceLength = 1;
    uint elementCount = scheme.size();

    word targetMask = scheme[startPosition].getInversionMask();
    for(uint index = startPosition + 1; index < elementCount; ++index)
    {
        if(scheme[index].getInversionMask() == targetMask)
            ++sequenceLength;
        else
            break;

        if(sequenceLength > 2)
            break;
    }

    return sequenceLength;
}

PostProcessor::OptScheme PostProcessor::optimizeInversions(const OptScheme& scheme)
{
    Optimizations optimizations;
    prepareSchemeForOptimization(scheme, &optimizations);

    uint elementCount = scheme.size();
    for(uint index = 0; index < elementCount; ++index)
    {
        const ReverseElement& element = scheme[index];
        word controlMask = element.getControlMask();
        word inversionMask = element.getInversionMask();

        auto count = countNonZeroBits;
        if(count(controlMask) < 3 && count(inversionMask) > 0
            && (controlMask & inversionMask) == inversionMask)
        {
            uint sequenceLength = findInversedElementsSequence(scheme, index);
            if(sequenceLength < 3)
            {
                uint optIndex = index;
                while(sequenceLength--)
                {
                    OptimizationParams& optimization = optimizations[optIndex];
                    optimization.asis = false;
                    optimization.inversions = true;

                    ++optIndex;
                }
            }
        }
    }

    return applyOptimizations(scheme, optimizations);
}

PostProcessor::OptScheme PostProcessor::removeDuplicates(const OptScheme& scheme)
{
    OptScheme optimizedScheme = scheme;
    int startIndex = 0;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectEqual,
            swapEqualElements, &repeat, false, true, &startIndex);
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::mergeOptimization(OptScheme& scheme, bool* optimized)
{
    auto selectFunc = (
        isNegativeControlInputsAllowed ?
        selectForMergeOptimization :
        selectForMergeOptimizationWithoutInversions
    );

    return generalOptimization(scheme, optimized, selectFunc,
        swapElementsWithMerge, false, false);
}

PostProcessor::OptScheme PostProcessor::reduceConnectionsOptimization( OptScheme& scheme, bool* optimized)
{
    return generalOptimization(scheme, optimized, selectForReduceConnectionsOptimization,
        swapElementsWithConnectionReduction, false, false);
}

PostProcessor::OptScheme PostProcessor::transferOptimization(OptScheme& scheme, bool* optimized)
{
    return generalOptimization(scheme, optimized, selectForTransferOptimization,
        swapElementsWithTransferOptimization, false, true);
}

PostProcessor::OptScheme PostProcessor::peresGateOptimization(OptScheme& scheme)
{
    OptScheme optimizedScheme = scheme;
    int startIndex = 0;
    bool repeat = true;

    while (repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectForPeresGateOptimization,
            swapPeresElements, &repeat, false, false, &startIndex, false);
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::generalOptimization(OptScheme& scheme,
    bool* optimized, SelectionFunc selectFunc, SwapFunc swapFunc,
    bool searchPairFromEnd, bool lessComplexityRequired, bool useNeighborElements /*= true*/)
{
    assertd(optimized, string("Null 'optimized' pointer (PostProcessor::generalOptimization)"));
    *optimized = false;

    OptScheme optimizedScheme = scheme;
    const uint numMaxSubSchemeSize = ProgramOptions::get().maxSubSchemeSizeForOptimization;

    bool repeatOuter = true;
    while (repeatOuter)
    {
        repeatOuter = false;

        uint schemeSize = optimizedScheme.size();
        uint stepCount = (schemeSize + numMaxSubSchemeSize - 1) / numMaxSubSchemeSize;

        OptScheme::const_iterator first = optimizedScheme.cbegin();
        uint currentProgress = 0;

        OptScheme tempScheme;
        for (uint step = 0; step < stepCount; ++step)
        {
            OptScheme::const_iterator last;
            if (currentProgress + numMaxSubSchemeSize <= schemeSize)
                last = first + numMaxSubSchemeSize;
            else
                last = first + (schemeSize - currentProgress);

            OptScheme subScheme(first, last);

            bool repeatInner = true;
            while (repeatInner)
            {
                subScheme = tryOptimizationTactics(subScheme, selectFunc,
                    swapFunc, &repeatInner, searchPairFromEnd,
                    lessComplexityRequired, 0, useNeighborElements);

                *optimized = *optimized || repeatInner;
                repeatOuter = repeatOuter || repeatInner;
            }

            tempScheme.insert(tempScheme.end(), subScheme.cbegin(), subScheme.cend());

            currentProgress += numMaxSubSchemeSize;
            first = last;
        }

        optimizedScheme = tempScheme;
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::getFullScheme(const OptScheme& scheme,
    FullSchemeType type, bool heavyRight /*= true*/)
{
    OptScheme fullScheme;
    uint elementCount = scheme.size();

    for(uint index = 0; index < elementCount; ++index)
    {
        const ReverseElement& element = scheme[index];
        deque<ReverseElement> impl;
        
        switch (type)
        {
        case fstSimple:
            impl = element.getSimpleImplementation();
            break;

        case fstRecursive:
            impl = element.getRecursiveImplementation();
            break;

        case fstToffoli:
            impl = element.getToffoliOnlyImplementation(heavyRight);
            break;

        default:
            assert(false, string("Dead code in switch"));
        }

        bufferize(fullScheme);
        fullScheme.insert(fullScheme.end(), impl.begin(), impl.end());
    }

    return fullScheme;
}

PostProcessor::OptScheme PostProcessor::getFinalSchemeImplementation(const OptScheme& scheme)
{
    OptScheme implementation;
    
    uint elementCount = scheme.size();
    implementation.resize(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        const ReverseElement& element = scheme[index];
        implementation[index] = element.getFinalImplementation();
    }

    return implementation;
}

//////////////////////////////////////////////////////////////////////////
// Main optimization tactic implementation
//////////////////////////////////////////////////////////////////////////

PostProcessor::OptScheme PostProcessor::tryOptimizationTactics(const OptScheme& scheme,
    SelectionFunc selectionFunc, SwapFunc swapFunc,
    bool* optimizationSucceeded, bool searchPairFromEnd,
    bool lessComplexityRequired, int* startIndex /* = 0 */, bool useNeighborElements /*= true*/)
{
    assertd(optimizationSucceeded,
        string("Null ptr 'optimizationSucceeded' (PostProcessor::tryOptimizationTactics)"));
    *optimizationSucceeded = false;

    OptScheme optimizedScheme = scheme;

    bool schemeOptimized = false;
    int elementCount = scheme.size();

    // find left element index
    for (int leftIndex = startIndex ? *startIndex : 0;
        !schemeOptimized && leftIndex < elementCount - 1; ++leftIndex)
    {
        if (startIndex)
            *startIndex = leftIndex;

        if (!useNeighborElements)
        {
            const ReverseElement& element = scheme[leftIndex];
            if (leftIndex > 0 && selectionFunc(element, scheme[leftIndex - 1]))
                continue;

            if (selectionFunc(element, scheme[leftIndex + 1]))
                continue;
        }

        uint leftElementMaxTransferIndex = uintUndefined;

        // find right element index
        for (int rightIndex = (searchPairFromEnd ? elementCount - 1 : leftIndex + 1);
            !schemeOptimized && (searchPairFromEnd ? rightIndex > leftIndex : rightIndex < elementCount);
            rightIndex += (searchPairFromEnd ? -1 : 1))
        {
            if ((uint)abs(leftIndex - rightIndex) > ProgramOptions::get().maxElementsDistanceForOptimization)
                break;

            if (!useNeighborElements && abs(leftIndex - rightIndex) == 1)
                continue;

            uint newLeftIndex  = uintUndefined;
            uint newRightIndex = uintUndefined;

            bool isElementsPairGoodForOptimization = false;
            if (ProgramOptions::get().options.getBool("use-swap-results-optimization-technique"))
            {
                SwapResultsPair pair = getSwapResultsPair(scheme, leftIndex, rightIndex);

                isElementsPairGoodForOptimization =
                    isSwapResultsPairSuiteOptimizationTactics(selectionFunc, pair,
                    leftIndex, rightIndex, &newLeftIndex, &newRightIndex);
            }
            else
            {
                const ReverseElement& left = scheme[leftIndex];
                const ReverseElement& right = scheme[rightIndex];

                // 1) check right element with selection function
                if (!selectionFunc(left, right))
                    continue;

                newLeftIndex = (uint)leftIndex;
                newRightIndex = (uint)rightIndex;

                // transfer elements if needed
                if (newLeftIndex + 1 != newRightIndex)
                {
                    // transfer right element to left at maximum
                    newRightIndex = getMaximumTransferIndex(scheme, right, rightIndex, leftIndex);
                    if (newRightIndex != leftIndex + 1)
                    {
                        // transfer left element to left at maximum (just once)
                        if (leftElementMaxTransferIndex == uintUndefined)
                        {
                            leftElementMaxTransferIndex = getMaximumTransferIndex(scheme, left,
                                leftIndex, elementCount - 1);
                        }

                        newLeftIndex = leftElementMaxTransferIndex;
                    }

                    // compare indices
                    if (newLeftIndex + 1 >= newRightIndex)
                        // good right element, try to apply optimization
                        newLeftIndex = newRightIndex - 1; // keep left element maximum left aligned
                    else
                        continue;
                }

                isElementsPairGoodForOptimization = true;
            }
        
            if (isElementsPairGoodForOptimization)
            {
                // move elements to new positions
                if (newLeftIndex < (uint)rightIndex)
                {
                    moveElementInScheme(&optimizedScheme, leftIndex, newLeftIndex);
                    moveElementInScheme(&optimizedScheme, rightIndex, newRightIndex);
                }
                else
                {
                    moveElementInScheme(&optimizedScheme, rightIndex, newRightIndex);
                    moveElementInScheme(&optimizedScheme, leftIndex, newLeftIndex);
                }

                // apply swap function
                const ReverseElement& leftElement = optimizedScheme[newLeftIndex];
                list<ReverseElement> leftReplacement;

                const ReverseElement& rightElement = optimizedScheme[newRightIndex];
                list<ReverseElement> rightReplacement;

                swapFunc(leftElement, rightElement, &leftReplacement, &rightReplacement);

                // replace elements with swap result
                Optimizations optimizations;
                prepareSchemeForOptimization(optimizedScheme, &optimizations);

                optimizations[newLeftIndex].replace = true;
                optimizations[newLeftIndex].replacement = leftReplacement;

                optimizations[newRightIndex].replace = true;
                optimizations[newRightIndex].replacement = rightReplacement;

                optimizedScheme = applyOptimizations(optimizedScheme, optimizations);
                if (optimizedScheme.size() >= scheme.size())
                {
                    // now try to optimize this scheme
                    optimizedScheme = removeDuplicates(optimizedScheme);

                    if (isNegativeControlInputsAllowed)
                    {
                        bool optimized;
                        optimizedScheme = mergeOptimization(optimizedScheme, &optimized);
                    }

                    if (lessComplexityRequired)
                        schemeOptimized = optimizedScheme.size() < scheme.size();
                    else
                        schemeOptimized = true;
                }
                else
                    schemeOptimized = true;

                if (!schemeOptimized)
                    optimizedScheme = scheme;
            }
        }
    }

    *optimizationSucceeded = schemeOptimized;
    return optimizedScheme;
}

deque<PostProcessor::SwapResult> PostProcessor::getSwapResult(OptScheme* scheme,
    uint startIndex, bool toLeft)
{
    deque<SwapResult> result;

    uint index = startIndex;
    uint stopIndex = 0;
    uint step = (uint)-1;

    if (!toLeft)
    {
        stopIndex = scheme->size() - 1;
        step = 1;
    }

    if (index == stopIndex)
        return result;

    Range range = { index, index };
    ReverseElement target = (*scheme)[index];

    do
    {
        index += step;
        ReverseElement& another = (*scheme)[index];

        bool withOneControlLineInverting = false;
        if (target.isSwappable(another, &withOneControlLineInverting, !isNegativeControlInputsAllowed))
        {
            // swap elements
            ReverseElement& targetCopy = (*scheme)[index - step];
            targetCopy.swap(&another);

            if (!withOneControlLineInverting)
                range.end += step;
            else
            {
                // remember current swap result
                range.sort();
                SwapResult sr = { target, range };

                if (toLeft)
                    result.push_front(sr);
                else
                    result.push_back(sr);

                // create new range
                range = { index, index };
            }

            // save result for next iteration
            target = another;
        }
        else
            break;

    } while (index != stopIndex);

    // remember current swap result
    range.sort();
    SwapResult sr = { target, range };

    if (toLeft)
        result.push_front(sr);
    else
        result.push_back(sr);

    return result;
}

PostProcessor::SwapResultsPair PostProcessor::getSwapResultsPair(
    const OptScheme& scheme, uint leftIndex, uint rightIndex)
{
    uint schemeSize = scheme.size();
    assertd(leftIndex < schemeSize && rightIndex < schemeSize,
        string("Wrong indices (PostProcessor::getSwapResultsPair)"));

    // TODO: check if this would be necessary
    assertd(leftIndex < rightIndex, string("Unordered indices (PostProcessor::getSwapResultsPair)"));

    // move to left
    OptScheme schemeCopy = scheme;
    deque<SwapResult> forLeftToLeft  = getSwapResult(&schemeCopy, leftIndex,  true);
    deque<SwapResult> forRightToLeft = getSwapResult(&schemeCopy, rightIndex, true);

    // move to right
    schemeCopy = scheme;
    deque<SwapResult> forRightToRight = getSwapResult(&schemeCopy, rightIndex, false);
    deque<SwapResult> forLeftToRight  = getSwapResult(&schemeCopy, leftIndex,  false);

    // merge results
    SwapResultsPair result;
    result.forLeft = mergeSwapResults(forLeftToLeft, forLeftToRight);
    result.forRight = mergeSwapResults(forRightToLeft, forRightToRight);

    return result;
}

deque<PostProcessor::SwapResult> PostProcessor::mergeSwapResults(
    deque<SwapResult>& toLeft, deque<SwapResult>& toRight)
{
    uint leftSize  = toLeft.size();
    uint rightSize = toRight.size();

    deque<SwapResult> merged;

    if (leftSize == 0)
        return toRight;
    else if (rightSize == 0)
        return toLeft;

    assertd(toLeft.size() && toRight.size(), string("Can't merge empty swap results"));

    SwapResult left = toLeft.back();
    toLeft.pop_back();

    SwapResult right = toRight.front();
    toRight.pop_front();

    assertd(left.first == right.first && left.second.end == right.second.start,
        string("Can't merge not neighbour swap results"));

    left.second.end = right.second.end;

    merged = toLeft;
    merged.push_back(left);

    for (auto& x : toRight)
        merged.push_back(x);

    return merged;
}

bool PostProcessor::isSwapResultsPairSuiteOptimizationTactics(
    SelectionFunc selectionFunc, const SwapResultsPair& result,
    uint leftIndex, uint rightIndex,
    uint* newLeftIndex, uint* newRightIndex)
{
    assertd(newLeftIndex && newRightIndex, string("Null ptr "
        "(PostProcessor::isSwapResultsPairSuiteOptimizationTactics)"));

    assertd(result.forLeft.size() && result.forRight.size(),
        string("Wrong swap result (PostProcessor::isSwapResultsPairSuiteOptimizationTactics)"));

    uint minDistance = uintUndefined;
    bool answer = false;

    for (auto& left : result.forLeft)
    {
        for (auto& right : result.forRight)
        {
            if (selectionFunc(left.first, right.first))
            {
                // found, check ranges
                Range range = right.second;
                if (range.start)
                    --range.start;

                if (range.end)
                    --range.end;

                for (uint index = left.second.start; index != left.second.end; ++index)
                {
                    if (range.has(index) && right.second.has(index + 1))
                    {
                        uint newLeft = index;
                        uint newRight = index + 1;

                        uint distance = abs(int(leftIndex - newLeft)) + abs(int(rightIndex - newRight));
                        if (distance < minDistance)
                        {
                            *newLeftIndex  = newLeft;
                            *newRightIndex = newRight;

                            minDistance = distance;
                            answer = true;
                        }
                    }
                }
            }
        }
    }

    return answer;
}

void PostProcessor::moveElementInScheme(OptScheme* scheme,
    uint fromIndex, uint toIndex)
{
    assertd(scheme, string("Null ptr (PostProcessor::moveElementInScheme)"));

    uint size = scheme->size();
    assertd(fromIndex < size && toIndex < size,
        string("Moving element out of borders (PostProcessor::moveElementInScheme)"));

    uint step = 1;
    if (toIndex < fromIndex)
        step = -1;

    while (fromIndex != toIndex)
    {
        ReverseElement& first  = (*scheme)[fromIndex];
        ReverseElement& second = (*scheme)[fromIndex + step];

        first.swap(&second);
        fromIndex += step;
    }
}

uint PostProcessor::getMaximumTransferIndex(const OptScheme& scheme,
    const ReverseElement& target, uint startIndex, uint stopIndex) const
{
    uint elementCount = (uint)scheme.size();

    assertd(startIndex < elementCount,
        string("PostProcessor: invalid start index for getMaximumTransferIndex()"));

    assertd(stopIndex < elementCount,
        string("PostProcessor: invalid stop index for getMaximumTransferIndex()"));

    int step = 1;
    if (startIndex > stopIndex)
        // from right to left
        step = -1;

    uint index = startIndex;
    while (index != stopIndex)
    {
        const ReverseElement& neighborElement = scheme[index];
        if (!target.isSwappable(neighborElement))
            // stop search if not swappable
            break;

        index += step;
    }

    index -= step; // target element can be in previous position, not in the last
    return index;
}

}   // namespace ReversibleLogic
