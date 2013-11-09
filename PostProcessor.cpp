#include "std.hpp"

namespace ReversibleLogic
{

PostProcessor::Optimizations::Optimizations()
    : inversions(false)
    , heavyRight(false)
    , remove(false)
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

    bool needOptimization = true;
    while(needOptimization)
    {
        optimizedScheme = mergeOptimization(optimizedScheme, &needOptimization);
    }

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

    //lengthBefore = implementation.size();
    //lengthAfter  = uintUndefined;
    //while(lengthBefore != lengthAfter)
    //{
    //    lengthBefore = implementation.size();
    //    implementation = removeDuplicates(implementation);
    //    lengthAfter = implementation.size();
    //}

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
            // leave element as is
            if(optimization.asis)
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
    bool* optimized /* = 0 */)
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

        ////ReverseElement& firstElement = scheme[firstIndex];
        ////for(uint secondIndex = firstIndex + 1; secondIndex < elementCount; ++secondIndex)
        ////{
        ////    const ReverseElement& secondElement = scheme[secondIndex];
        ////    Optimizations& secondOptimization = optimizations[secondIndex];

        ////    if(!firstElement.isSwitchable(secondElement)
        ////        || secondOptimization.remove)
        ////    {
        ////        break;
        ////    }

        ////    if(firstElement.getTargetMask() == secondElement.getTargetMask()
        ////        && firstElement.getControlMask() == secondElement.getControlMask())
        ////    {
        ////        word firstInversionMask  =  firstElement.getInversionMask();
        ////        word secondInversionMask = secondElement.getInversionMask();

        ////        word differentInversionMask = firstInversionMask ^ secondInversionMask;

        ////        if(countNonZeroBits(differentInversionMask) == 1)
        ////        {
        ////            word firstControlMask = firstElement.getControlMask();
        ////            assert((differentInversionMask & firstControlMask), string("Internal error"));

        ////            firstControlMask ^= differentInversionMask;
        ////            firstElement.setControlMask(firstControlMask);

        ////            if(differentInversionMask & firstInversionMask)
        ////            {
        ////                firstInversionMask ^= differentInversionMask;
        ////                firstElement.setInversionMask(firstInversionMask);
        ////            }

        ////            secondOptimization.remove = true;
        ////            break;
        ////        }
        ////    }
        ////}


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
                else if(differentInvesionBitsCount > 1)
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
                        // change first
                        word firstControlMask = firstElement.getControlMask();
                        firstControlMask &= ~(firstInversionMask & removeMask);
                        firstElement.setControlMask(firstControlMask);

                        firstInversionMask &= ~removeMask;
                        firstElement.setInversionMask(firstInversionMask);

                        firstOptimization.asis  = false;

                        // change second
                        word secondControlMask = secondElement.getControlMask();
                        secondControlMask &= ~(secondInversionMask & removeMask);
                        secondElement.setControlMask(secondControlMask);

                        secondInversionMask &= ~removeMask;
                        secondElement.setInversionMask(secondInversionMask);

                        secondOptimization.asis = false;

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

}   // namespace ReversibleLogic
