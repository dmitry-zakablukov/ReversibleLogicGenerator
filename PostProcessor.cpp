#include "std.hpp"

namespace ReversibleLogic
{

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

    prepareSchemeForOptimization(scheme);

    uint elementCount = scheme.size();
    for(uint firstIndex = 0; firstIndex < elementCount; ++firstIndex)
    {
        Optimizations& firstOptimization = optimizations[firstIndex];
        if(!firstOptimization.asis)
        {
            continue;
        }

        ReverseElement& firstElement = scheme[firstIndex];
        for(uint secondIndex = firstIndex + 1; secondIndex < elementCount; ++secondIndex)
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
                uint firstElementCheckCount = countNonZeroBits(firstInversionMask & differentInversionMask);

                // check first element inversion mask to exclude situation with
                // (00)(11)
                if(differentInvesionBitsCount == 2 && firstElementCheckCount == 1)
                {
                    // (01)(10) -> (*1)(1*)

                    uint firstRemovePosition = findPositiveBitPosition(differentInversionMask);
                    word firstRemoveMask = 1 << firstRemovePosition;

                    uint secondRemovePosition =
                        findPositiveBitPosition(differentInversionMask, firstRemovePosition + 1);
                    word secondRemoveMask = 1 << secondRemovePosition;
                    word removeMask = firstRemoveMask ^ secondRemoveMask;

                    if((firstInversionMask & removeMask) != removeMask
                        && (secondInversionMask & removeMask) != removeMask )
                    {
                        // change first element
                        word firstControlMask = firstElement.getControlMask();
                        firstControlMask &= ~(firstInversionMask & removeMask);
                        firstElement.setControlMask(firstControlMask);

                        firstInversionMask &= ~removeMask;
                        firstElement.setInversionMask(firstInversionMask);

                        // change second element
                        word secondControlMask = secondElement.getControlMask();
                        secondControlMask &= ~(secondInversionMask & removeMask);
                        secondElement.setControlMask(secondControlMask);

                        secondInversionMask &= ~removeMask;
                        secondElement.setInversionMask(secondInversionMask);

                        // first element was transfered to second, so remove it from
                        // previous position and stick to the second element
                        firstOptimization.remove = true;

                        secondOptimization.replace = true;

                        vector<ReverseElement>& replacement = secondOptimization.replacement;
                        replacement.resize(2);
                        
                        replacement[0] = firstElement;
                        replacement[1] = secondElement;

                        *optimized = true;
                        break;
                    }
                }
            }
        }
    }

    return applyOptimizations(scheme);
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
    prepareSchemeForOptimization(scheme);

    uint elementCount = scheme.size();
    for(uint firstIndex = 0; firstIndex < elementCount; ++firstIndex)
    {
        Optimizations& firstOptimization = optimizations[firstIndex];
        if(firstOptimization.remove)
        {
            continue;
        }

        const ReverseElement& firstElement = scheme[firstIndex];
        for(uint secondIndex = firstIndex + 1; secondIndex < elementCount; ++secondIndex)
        {
            const ReverseElement& secondElement = scheme[secondIndex];
            if(firstElement == secondElement)
            {
                firstOptimization.remove = true;
                optimizations[secondIndex].remove = true;
                break;
            }
            else if(!firstElement.isSwitchable(secondElement))
            {

                break;
            }
        }
    }

    return applyOptimizations(scheme);
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

}   // namespace ReversibleLogic
