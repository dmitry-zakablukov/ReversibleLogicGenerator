#include "std.hpp"

namespace ReversibleLogic
{

//////////////////////////////////////////////////////////////////////////
// Selection functions
//////////////////////////////////////////////////////////////////////////
bool selectEqual(const ReverseElement& left, const ReverseElement& right)
{
    return left == right;
}

bool selectForReduceConnectionsOptimization(const ReverseElement& left,
    const ReverseElement& right)
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

//////////////////////////////////////////////////////////////////////////
// Swap functions
//////////////////////////////////////////////////////////////////////////
void swapEqualElements(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assert(selectEqual(left, right), string("swapEqualElements(): wrong input elements"));

    // leave replacements empty
    leftReplacement->resize(0);
    rightReplacement->resize(0);
}

void swapElementsWithConnectionReduction(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assert(selectForReduceConnectionsOptimization(left, right),
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

PostProcessor::Optimizations::Optimizations()
    : inversions(false)
    , heavyRight(false)
    , remove(false)
    , replace(false)
    , replacement()
    , asis(true)
{
}

PostProcessor::PostProcessor()
    : optimizations()
{
}

PostProcessor::Scheme PostProcessor::optimize(const Scheme& scheme)
{
    Scheme optimizedScheme = scheme;
    uint lengthBefore = uintUndefined;
    uint lengthAfter  = uintUndefined;
    
    //// debug
    //return scheme;

    ////// optimize inversions
    ////optimizedScheme = optimizeInversions(optimizedScheme);

    ////// merge optimization
    ////lengthBefore = optimizedScheme.size();
    ////lengthAfter  = uintUndefined;

    ////lengthBefore = optimizedScheme.size();
    ////lengthAfter  = uintUndefined;
    ////while(lengthBefore != lengthAfter)
    ////{
    ////    lengthBefore = optimizedScheme.size();
    ////    optimizedScheme = removeDuplicates(optimizedScheme);
    ////    lengthAfter = optimizedScheme.size();
    ////}

    ////optimizedScheme = removeDuplicates(optimizedScheme);

    bool needOptimization = true;
    uint step = 0;
    while(needOptimization /*&& step < 1*/)
    {
        optimizedScheme = mergeOptimization(optimizedScheme, false, &needOptimization);
        bool additionalOptimized = false;
        optimizedScheme = mergeOptimization(optimizedScheme, true, &additionalOptimized);

        needOptimization = needOptimization || additionalOptimized;
        additionalOptimized = true;
        while(additionalOptimized)
        {
            optimizedScheme = reduceConnectionsOptimization(optimizedScheme, &additionalOptimized);
        }

        needOptimization = needOptimization || additionalOptimized;
        ++step;
    }

    //needOptimization = true;
    //while(needOptimization)
    //{
    //    optimizedScheme = reduceConnectionsOptimization(optimizedScheme, &needOptimization);
    //}

    ////lengthBefore = optimizedScheme.size();
    ////lengthAfter  = uintUndefined;
    ////while(lengthBefore != lengthAfter)
    ////{
    ////    lengthBefore = optimizedScheme.size();
    ////    optimizedScheme = removeDuplicates(optimizedScheme);
    ////    lengthAfter = optimizedScheme.size();
    ////}

    ////needOptimization = true;
    ////while(needOptimization)
    ////{
    ////    optimizedScheme = mergeOptimization(optimizedScheme, &needOptimization);
    ////}

    ////// remove duplicates
    ////optimizedScheme = removeDuplicates(optimizedScheme);

    // final implementation
    //implementation = self.__getFinalSchemeImplementation(optimizedScheme)
    Scheme implementation;
    implementation = optimizedScheme;

    implementation = getFullScheme(optimizedScheme);

    ////lengthBefore = implementation.size();
    ////lengthAfter  = uintUndefined;
    ////while(lengthBefore != lengthAfter)
    ////{
    ////    lengthBefore = implementation.size();
    ////    implementation = removeDuplicates(implementation);
    ////    lengthAfter = implementation.size();
    ////}

    needOptimization = true;
    uint startPos = 0;
    //step = 0;
    while(needOptimization/* && step < 1*/)
    {
        startPos = 0;
        implementation = transferOptimization(implementation, &startPos, false, &needOptimization);
        bool additionalOptimized = false;
        startPos = 0;
        implementation = transferOptimization(implementation, &startPos, true, &additionalOptimized);

        needOptimization = needOptimization || additionalOptimized;


        implementation = removeDuplicates(implementation);
        //++step;
    }    

    ////implementation = getFullScheme(implementation);

    ////lengthBefore = implementation.size();
    ////lengthAfter  = uintUndefined;
    ////while(lengthBefore != lengthAfter)
    ////{
    ////    lengthBefore = implementation.size();
    ////    implementation = removeDuplicates(implementation);
    ////    lengthAfter = implementation.size();
    ////}

    return implementation;
}

PostProcessor::Optimizations& PostProcessor::getOptimization(uint index)
{
    return optimizations[index];
}

void PostProcessor::prepareSchemeForOptimization(const Scheme& scheme)
{
    uint elementCount = scheme.size();
    optimizations.resize(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        optimizations[index] = Optimizations();
    }
}

PostProcessor::Scheme PostProcessor::applyOptimizations(const Scheme& scheme)
{
    uint elementCount = scheme.size();
    Scheme optimizedScheme;
    optimizedScheme.reserve(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        const Optimizations& optimization = optimizations[index];
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
            {
                optimizedScheme.insert(optimizedScheme.end(), scheme[index]);
            }
            else
            {
                if(optimization.inversions) //inversions
                {
                    bool heavyRight = optimization.heavyRight;
                    auto impl = scheme[index].getInversionOptimizedImplementation(heavyRight);
                    optimizedScheme.insert(optimizedScheme.end(), impl.begin(), impl.end());
                }
                else //leave element as is
                {
                    optimizedScheme.insert(optimizedScheme.end(), scheme[index]);
                }
            }
        }
    }

    return optimizedScheme;
}

uint PostProcessor::findInversedElementsSequence(const Scheme& scheme, uint startPosition)
{
    uint sequenceLength = 1;
    uint elementCount = scheme.size();

    word targetMask = scheme[startPosition].getInversionMask();
    for(uint index = startPosition + 1; index < elementCount; ++index)
    {
        if(scheme[index].getInversionMask() == targetMask)
        {
            ++sequenceLength;
        }
        else
        {
            break;
        }

        if(sequenceLength > 2)
        {
            break;
        }
    }

    return sequenceLength;
}

PostProcessor::Scheme PostProcessor::optimizeInversions(const Scheme& scheme)
{
    prepareSchemeForOptimization(scheme);

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
                    Optimizations& optimization = optimizations[optIndex];
                    optimization.asis = false;
                    optimization.inversions = true;

                    ++optIndex;
                }
            }
        }
    }

    return applyOptimizations(scheme);
}

PostProcessor::Scheme PostProcessor::mergeOptimization(Scheme& scheme,
    bool reverse, bool* optimized /* = 0 */)
{
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    prepareSchemeForOptimization(scheme);

    uint elementCount = scheme.size();

    uint startIndex = (reverse ? elementCount - 1 : 0);
    uint stopIndex  = (reverse ? -1 : elementCount);
    uint step = (reverse ? -1 : 1);

    for(uint firstIndex = startIndex; firstIndex != stopIndex; firstIndex += step)
    {
        Optimizations& firstOptimization = optimizations[firstIndex];
        if(!firstOptimization.asis)
        {
            continue;
        }

        ReverseElement& firstElement = scheme[firstIndex];
        for(uint secondIndex = firstIndex + step; secondIndex != stopIndex; secondIndex += step)
        {
            ReverseElement& secondElement = scheme[secondIndex];
            Optimizations& secondOptimization = optimizations[secondIndex];

            if(!firstElement.isSwitchable(secondElement))
            {
                break;
            }

            if(!secondOptimization.asis)
            {
                continue;
            }

            if(firstElement.getTargetMask() == secondElement.getTargetMask()
                && firstElement.getControlMask() == secondElement.getControlMask())
            {
                word firstInversionMask  =  firstElement.getInversionMask();
                word secondInversionMask = secondElement.getInversionMask();

                word differentInversionMask = firstInversionMask ^ secondInversionMask;

                uint differentInvesionBitsCount = countNonZeroBits(differentInversionMask);
                if(differentInvesionBitsCount == 1)
                {
                    // (01)(11) -> (1)

                    word secondControlMask = secondElement.getControlMask();
                    assert((differentInversionMask & secondControlMask), string("Internal error"));

                    secondControlMask ^= differentInversionMask;
                    secondElement.setControlMask(secondControlMask);

                    if(differentInversionMask & secondInversionMask)
                    {
                        secondInversionMask ^= differentInversionMask;
                        secondElement.setInversionMask(secondInversionMask);
                    }

                    firstOptimization.remove = true;
                    firstOptimization.asis  = false;
                    secondOptimization.asis = false;

                    *optimized = true;
                    break;
                }
            }
        }
    }

    return applyOptimizations(scheme);
}

PostProcessor::Scheme PostProcessor::reduceConnectionsOptimization( Scheme& scheme, bool* optimized /*= 0 */ )
{
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    Scheme optimizedScheme = scheme;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectForReduceConnectionsOptimization,
            swapElementsWithConnectionReduction, &repeat, false, false);

        *optimized = *optimized || repeat;
    }

    return optimizedScheme;
}

PostProcessor::Scheme PostProcessor::getFullScheme(const Scheme& scheme, bool heavyRight /*= true*/)
{
    Scheme fullScheme;
    uint elementCount = scheme.size();

    for(uint index = 0; index < elementCount; ++index)
    {
        const ReverseElement& element = scheme[index];
        auto impl = element.getImplementation(heavyRight);

        bufferize(fullScheme);
        fullScheme.insert(fullScheme.end(), impl.begin(), impl.end());
    }

    return fullScheme;
}

PostProcessor::Scheme PostProcessor::removeDuplicates(const Scheme& scheme)
{
    Scheme optimizedScheme = scheme;
    int startIndex = 0;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectEqual,
            swapEqualElements, &repeat, false, true, &startIndex);
    }

    return optimizedScheme;
}

PostProcessor::Scheme PostProcessor::getFinalSchemeImplementation(const Scheme& scheme)
{
    Scheme implementation;
    
    uint elementCount = scheme.size();
    implementation.resize(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        const ReverseElement& element = scheme[index];
        implementation[index] = element.getFinalImplementation();
    }

    return implementation;
}

PostProcessor::Scheme PostProcessor::transferOptimization(Scheme& scheme, uint* startPos,
    bool reverse, bool* optimized /*= 0 */)
{
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    uint elementCount = scheme.size();
    if(elementCount < 3 || *startPos > elementCount - 3)
    {
        // this optimization doesn't work on 2 or less elements
        return scheme;
    }

    prepareSchemeForOptimization(scheme);

    uint startIndex = (reverse ? elementCount - 1 - *startPos : *startPos);
    uint stopIndex  = (reverse ? 0 : elementCount - 1);
    uint step = (reverse ? -1 : 1);

    bool found = false;
    for(uint firstIndex = startIndex; firstIndex != stopIndex; firstIndex += step, ++*startPos)
    {
        ReverseElement& firstElement = scheme[firstIndex];
        assert(firstElement.getInversionMask() == 0,
            string("There should be no inversions for transfer optimization"));

        uint secondIndex = firstIndex + step;
        while(secondIndex != stopIndex)
        {
            ReverseElement& secondElement = scheme[secondIndex];
            if(!firstElement.isSwitchable(secondElement))
            {
                break;
            }

            secondIndex += step;
        }

        word firstTargetMask  = firstElement.getTargetMask();
        word firstControlMask = firstElement.getControlMask();

        uint targetIndex = secondIndex;
        while(targetIndex != stopIndex + step)
        {
            ReverseElement& targetElement = scheme[targetIndex];
            assert(targetElement.getInversionMask() == 0,
                string("There should be no inversions for transfer optimization"));

            // check if second element could be transfered to first element
            if(targetIndex != secondIndex)
            {
                uint index = targetIndex - step;
                while(index != firstIndex)
                {
                    ReverseElement& element = scheme[index];
                    if(!targetElement.isSwitchable(element))
                    {
                        break;
                    }

                    index -= step;
                }

                if(index != firstIndex)
                {
                    break;
                }
            }

            word targetTargetMask  = targetElement.getTargetMask();
            word targetControlMask = targetElement.getControlMask();

            if((firstTargetMask & targetControlMask) && !(targetTargetMask & firstControlMask))
            {
                uint n = max(firstElement.getInputCount(), targetElement.getInputCount());
                ReverseElement newElement(n, targetTargetMask,
                    firstControlMask | (targetControlMask ^ firstTargetMask));

                // search forward for copy of this element
                for(uint thirdIndex = secondIndex; thirdIndex != stopIndex + step; thirdIndex += step)
                {
                    ReverseElement& thirdElement = scheme[thirdIndex];
                    if(newElement == thirdElement)
                    {
                        if(targetIndex == secondIndex)
                        {
                            optimizations[firstIndex ].remove = true;
                            optimizations[thirdIndex ].remove = true;

                            Optimizations& secondOptimization = optimizations[secondIndex];
                            secondOptimization.replace = true;

                            vector<ReverseElement>& replacement = secondOptimization.replacement;
                            replacement.resize(2);
                            if(reverse)
                            {
                                replacement[0] = firstElement;
                                replacement[1] = targetElement;
                            }
                            else
                            {
                                replacement[0] = targetElement;
                                replacement[1] = firstElement;
                            }
                        }
                        else
                        {
                            if(secondIndex != thirdIndex)
                            {
                                optimizations[firstIndex ].remove = true;
                                optimizations[targetIndex].remove = true;
                                optimizations[thirdIndex ].remove = true;

                                Optimizations& secondOptimization = optimizations[secondIndex];
                                secondOptimization.replace = true;

                                vector<ReverseElement>& replacement = secondOptimization.replacement;
                                replacement.resize(3);
                                if(reverse)
                                {
                                    replacement[0] = scheme[secondIndex];
                                    replacement[1] = firstElement;
                                    replacement[2] = targetElement;
                                }
                                else
                                {
                                    replacement[0] = targetElement;
                                    replacement[1] = firstElement;
                                    replacement[2] = scheme[secondIndex];
                                }
                            }
                            else
                            {
                                optimizations[firstIndex ].remove = true;
                                optimizations[targetIndex].remove = true;

                                Optimizations& secondOptimization = optimizations[secondIndex];
                                secondOptimization.replace = true;

                                vector<ReverseElement>& replacement = secondOptimization.replacement;
                                replacement.resize(2);
                                replacement[0] = targetElement;
                                replacement[1] = firstElement;
                            }
                        }

                        *optimized = true;
                        found = true;
                        break;
                    }
                    else if(newElement.isSwitchable(thirdElement))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                ////if(!found)
                ////{
                ////    // search backward for copy of this element
                ////    for(uint thirdIndex = targetIndex - step; thirdIndex != firstIndex; thirdIndex -= step)
                ////    {
                ////        ReverseElement& thirdElement = scheme[thirdIndex];
                ////        if(newElement == thirdElement)
                ////        {
                ////            Optimizations& firstOptimization = optimizations[firstIndex];
                ////            firstOptimization.replace = true;

                ////            vector<ReverseElement>& replacement = firstOptimization.replacement;
                ////            replacement.resize(2);
                ////            replacement[0] = targetElement;
                ////            replacement[1] = firstElement;

                ////            optimizations[targetIndex].remove = true;
                ////            optimizations[thirdIndex ].remove = true;

                ////            *optimized = true;
                ////            found = true;
                ////            break;
                ////        }
                ////        else if(newElement.isSwitchable(thirdElement))
                ////        {
                ////            continue;
                ////        }
                ////        else
                ////        {
                ////            break;
                ////        }
                ////    }
                ////}
            }

            if(found)
            {
                break;
            }

            targetIndex += step;
        }

        if(found)
        {
            break;
        }
    }

    return applyOptimizations(scheme);
}

//////////////////////////////////////////////////////////////////////////
// Main optimization tactic implementation
//////////////////////////////////////////////////////////////////////////

PostProcessor::Scheme PostProcessor::tryOptimizationTactics(const Scheme& scheme,
    SelectionFunc selectionFunc, SwapFunc swapFunc,
    bool* optimizationSucceeded, bool searchPairFromEnd,
    bool lessComplexityRequired, int* startIndex /* = 0 */)
{
    prepareSchemeForOptimization(scheme);

    bool schemeOptimized = false;

    int elementCount = scheme.size();
    
    // find left element
    for(int leftIndex = startIndex ? *startIndex : 0;
        !schemeOptimized && leftIndex < elementCount - 1; ++leftIndex)
    {
        if(startIndex)
        {
            *startIndex = leftIndex;
        }

        int leftElementMaxTransferIndex = -1; // for optimization
        const ReverseElement& leftElement = scheme[leftIndex];

        // find right element
        for(int rightIndex = (searchPairFromEnd ? elementCount - 1 : leftIndex + 1);
            !schemeOptimized && (searchPairFromEnd ? rightIndex > leftIndex : rightIndex < elementCount);
            rightIndex += (searchPairFromEnd ? -1 : 1))
        {
            const ReverseElement& rightElement = scheme[rightIndex];

            // 1) check right element with selection function
            if(selectionFunc(leftElement, rightElement))
            {
                int transferedLeftIndex  = leftIndex;
                int transferedRightIndex = rightIndex;

                // transfer elements if needed
                if(transferedLeftIndex + 1 != transferedRightIndex)
                {
                    // transfer right element to left at maximum
                    transferedRightIndex = getMaximumTransferIndex(scheme, rightElement, rightIndex, leftIndex);

                    if(transferedRightIndex != leftIndex + 1)
                    {
                        // transfer left element to left at maximum (just once)
                        if(leftElementMaxTransferIndex == -1)
                        {
                            leftElementMaxTransferIndex = getMaximumTransferIndex(scheme, leftElement,
                                leftIndex, elementCount - 1);
                        }

                        transferedLeftIndex = leftElementMaxTransferIndex;
                    }

                    // compare indices
                    if(transferedLeftIndex + 1 >= transferedRightIndex)
                    {
                        // good right element, try to apply optimization
                        transferedLeftIndex = transferedRightIndex - 1; // keep left element maximum left aligned
                    }
                    else
                    {
                        continue;
                    }
                }

                // 2) swap left and right elements
                list<ReverseElement> leftReplacement;
                list<ReverseElement> rightReplacement;

                swapFunc(leftElement, rightElement, &leftReplacement, &rightReplacement);
                if(leftReplacement.size() || rightReplacement.size())
                {
                    bool duplicatesFound = processReplacements(scheme, leftIndex, transferedLeftIndex,
                        rightIndex, transferedRightIndex, leftReplacement, rightReplacement);

                    schemeOptimized = !lessComplexityRequired || duplicatesFound;
                }
                else
                {
                    // just remove left and right elements from scheme
                    Optimizations& leftOptimization = optimizations[leftIndex];
                    leftOptimization.remove = true;

                    Optimizations& rightOptimization = optimizations[rightIndex];
                    rightOptimization.remove = true;

                    schemeOptimized = true;
                }
            }
        }
    }

    if(optimizationSucceeded)
    {
        *optimizationSucceeded = schemeOptimized;
    }

    return applyOptimizations(scheme);
}

int PostProcessor::getMaximumTransferIndex(const Scheme& scheme,
    const ReverseElement& target, int startIndex, int stopIndex) const
{
    int elementCount = scheme.size();

    assert(startIndex >= 0 && startIndex < elementCount,
        string("PostProcessor: invalid start index for getMaximumTransferIndex()"));

    assert(stopIndex >= 0 && stopIndex < elementCount,
        string("PostProcessor: invalid stop index for getMaximumTransferIndex()"));

    int step = 1;
    if(startIndex > stopIndex)
    {
        // from right to left
        step = -1;
    }

    int index = startIndex;
    while(index != stopIndex)
    {
        const ReverseElement& neighborElement = scheme[index];

        // stop search if not switchable
        if(!target.isSwitchable(neighborElement))
        {
            break;
        }

        index += step;
    }

    index -= step; // target element can be in previous position, not in the last
    return index;
}

bool PostProcessor::processReplacements(const Scheme& scheme,
    int leftIndex, int transferedLeftIndex,
    int rightIndex, int transferedRightIndex,
    const list<ReverseElement>& leftReplacement,
    const list<ReverseElement>& rightReplacement)
{
    // 1) check replacements
    checkReplacement(leftReplacement);
    checkReplacement(rightReplacement);

    // 2) check element count in left and right replacement less than 4
    assert(leftReplacement.size() + rightReplacement.size() < 4,
        string("PostProcessor: too many elements in replacements"));

    // 3) find duplicates if needed
    bool onlyOneReplacement  = false;
    bool someDuplicatesFound = false;

    // left replacement processing
    vector<ReverseElement> leftProcessedReplacement;
    bool foundDuplicatesInLeftReplacement = false;

    if(leftReplacement.size())
    {
        foundDuplicatesInLeftReplacement =
            processDuplicatesInReplacement(scheme, leftReplacement, leftIndex,
            transferedLeftIndex, false, &leftProcessedReplacement);

        if(!rightReplacement.size())
        {
            foundDuplicatesInLeftReplacement = foundDuplicatesInLeftReplacement ||
                processDuplicatesInReplacement(scheme, leftReplacement, rightIndex,
                transferedRightIndex, true, &leftProcessedReplacement);

            onlyOneReplacement = true;
        }
    }

    // right replacement processing
    vector<ReverseElement> rightProcessedReplacement;
    bool foundDuplicatesInRightReplacement = false;

    if(rightReplacement.size())
    {
        foundDuplicatesInRightReplacement =
            processDuplicatesInReplacement(scheme, rightReplacement, rightIndex,
            transferedRightIndex, true, &rightProcessedReplacement);

        if(!leftReplacement.size())
        {
            foundDuplicatesInRightReplacement = foundDuplicatesInRightReplacement ||
                processDuplicatesInReplacement(scheme, rightReplacement, leftIndex,
                transferedRightIndex, false, &rightProcessedReplacement);

            onlyOneReplacement = true;
        }
    }

    someDuplicatesFound = foundDuplicatesInLeftReplacement || foundDuplicatesInRightReplacement;
    bool success = onlyOneReplacement || someDuplicatesFound;

    if(success)
    {
        setReplacement(scheme,  leftProcessedReplacement,  leftIndex,  transferedLeftIndex);
        setReplacement(scheme, rightProcessedReplacement, rightIndex, transferedRightIndex);
    }

    return success;
}

void PostProcessor::checkReplacement(const list<ReverseElement>& replacement)
{
    uint elementCount = replacement.size();
    if(elementCount > 1)
    {
        const ReverseElement& firstElement = replacement.front();
        word targetMask = firstElement.getTargetMask();

        forcin(element, replacement)
        {
            word mask = element->getTargetMask();
            assert(mask == targetMask, string("PostProcessor: invalid replacement list"));
        }
    }
}

int PostProcessor::findDuplicateElementIndex(const Scheme& scheme,
    const ReverseElement& target, int startIndex, int stopIndex, int skipIndex) const
{
    int elementCount = scheme.size();

    assert(startIndex >= 0 && startIndex < elementCount,
        string("PostProcessor: invalid start index for findDuplicateElementIndex()"));

    assert(stopIndex >= -1 && stopIndex <= elementCount,
        string("PostProcessor: invalid stop index for findDuplicateElementIndex()"));

    int step = 1;
    if(startIndex > stopIndex)
    {
        // from right to left
        step = -1;
    }

    int index = startIndex;
    bool found = false;

    while(index != stopIndex)
    {
        if(index != skipIndex)
        {
            const ReverseElement& neighborElement = scheme[index];

            // stop search if neighbor element is duplicate of target element
            if(target == neighborElement)
            {
                found = true;
                break;
            }

            // stop search if not switchable
            if(!target.isSwitchable(neighborElement))
            {
                break;
            }
        }

        index += step;
    }

    int resultIndex = (found ? index : -1);
    return resultIndex;
}

bool PostProcessor::processDuplicatesInReplacement(const Scheme& scheme,
    const list<ReverseElement>& replacement, int originalIndex,
    int transferedIndex, bool searchToRight,
    vector<ReverseElement>* processedReplacement)
{
    bool someDuplicatesFound = false;

    int elementCount = scheme.size();
    forcin(iter, replacement)
    {
        const ReverseElement& element = *iter;

        int duplicateIndex = findDuplicateElementIndex(scheme, element, transferedIndex,
            searchToRight ? elementCount : -1, originalIndex);

        if(duplicateIndex == -1)
        {
            processedReplacement->push_back(element);
        }
        else
        {
            Optimizations& duplicateOptimization = getOptimization(duplicateIndex);
            duplicateOptimization.remove = true;

            someDuplicatesFound = true;
        }
    }

    return someDuplicatesFound;
}

void PostProcessor::setReplacement(const Scheme& scheme,
    vector<ReverseElement>& replacement,
    int originalIndex, int transferedIndex)
{
    int elementCount = replacement.size();
    if(!elementCount)
    {
        // all replacement elements were removed as duplicates,
        // so remove original element from scheme
        Optimizations& targetOptimization = getOptimization(originalIndex);
        targetOptimization.remove = true;
    }
    else
    {
        if(originalIndex == transferedIndex)
        {
            // element wasn't transfered, just replace it
            Optimizations& targetOptimization = getOptimization(originalIndex);
            targetOptimization.replace      = true;
            targetOptimization.replacement  = replacement;
        }
        else
        {
            // remove original element
            Optimizations& targetOptimization = getOptimization(originalIndex);
            targetOptimization.remove = true;

            // replace element with transfered index with itself plus replacement
            const ReverseElement& transferedElement = scheme[transferedIndex];
            replacement.insert(replacement.begin(), transferedElement);

            Optimizations& transferedOptimization = getOptimization(transferedIndex);
            transferedOptimization.replace      = true;
            transferedOptimization.replacement  = replacement;
        }
    }
}

}   // namespace ReversibleLogic
