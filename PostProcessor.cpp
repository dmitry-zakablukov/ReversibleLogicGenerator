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

bool selectForMergeOptimization(const ReverseElement& left,
    const ReverseElement& right)
{
    // (01)(11) -> (*1)

    bool result = false;
    if(left.getTargetMask() == right.getTargetMask()
        && left.getControlMask() == right.getControlMask())
    {
        word  leftInversionMask =  left.getInversionMask();
        word rightInversionMask = right.getInversionMask();

        word diffInversionMask = leftInversionMask ^ rightInversionMask;
        result = (countNonZeroBits(diffInversionMask) == 1);
    }

    return result;
}

bool selectForTransferOptimization(const ReverseElement& left,
    const ReverseElement& right)
{
    word leftTargetMask   =  left.getTargetMask();
    word leftControlMask  =  left.getControlMask();

    word rightTargetMask  = right.getTargetMask();
    word rightControlMask = right.getControlMask();

    bool result = false;
    if(!left.isSwitchable(right))
    {
        // (left_target in right_controls) xor (right_target in left_controls)
        result = ((rightControlMask & leftTargetMask) != 0) ^ ((leftControlMask & rightTargetMask) != 0);
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

void swapElementsWithMerge(const ReverseElement& left, const ReverseElement& right,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assert(selectForMergeOptimization(left, right),
        string("swapElementsWithMerge(): wrong input elements"));

    word inversionMask = left.getInversionMask();
    word controlMask   = left.getControlMask();
    word diffInversionMask  = inversionMask ^ right.getInversionMask();

    // important! fill only left replacement
    ReverseElement element = left;
    element.setInversionMask(inversionMask & ~diffInversionMask);
    element.setControlMask(controlMask & ~diffInversionMask);

    leftReplacement->push_back(element);
}

void swapElementsWithTransferOptimization(const ReverseElement& leftElement, const ReverseElement& rightElement,
    list<ReverseElement>* leftReplacement, list<ReverseElement>* rightReplacement)
{
    assert(selectForTransferOptimization(leftElement, rightElement),
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

//////////////////////////////////////////////////////////////////////////
PostProcessor::OptimizationParams::OptimizationParams()
    : inversions(false)
    , heavyRight(false)
    , remove(false)
    , replace(false)
    , replacement()
    , asis(true)
{
}

PostProcessor::PostProcessor()
    : testScheme()
    , secondPassOptimizationFlag(true)
    , complexityDelta(0)
{
}

PostProcessor::OptScheme PostProcessor::optimize(const OptScheme& scheme)
{
    OptScheme optimizedScheme = scheme;
    uint lengthBefore = uintUndefined;
    uint lengthAfter  = uintUndefined;
    
    //// debug
    //return scheme;

    //optimizedScheme = removeDuplicates(optimizedScheme);

    bool needOptimization = true;
    uint step = 0;
    while(needOptimization /*&& step < 1*/)
    {
        needOptimization = false;
        optimizedScheme = mergeOptimization(optimizedScheme, &needOptimization);

        bool additionalOptimized = false;
        optimizedScheme = reduceConnectionsOptimization(optimizedScheme, &additionalOptimized);
        needOptimization = needOptimization || additionalOptimized;

        optimizedScheme = transferOptimization(optimizedScheme, &additionalOptimized);
        needOptimization = needOptimization || additionalOptimized;

        ++step;
    }

    // final implementation
    OptScheme implementation;
    implementation = optimizedScheme;

    implementation = getFullScheme(optimizedScheme);

    needOptimization = true;
    //step = 0;
    while(needOptimization/* && step < 1*/)
    {
        needOptimization = false;
        implementation = transferOptimization(implementation, &needOptimization);
        implementation = removeDuplicates(implementation);
        //++step;
    }    

    return implementation;
}

void PostProcessor::prepareSchemeForOptimization(const OptScheme& scheme,
    Optimizations* optimizations)
{
    uint elementCount = scheme.size();
    optimizations->resize(elementCount);

    for(uint index = 0; index < elementCount; ++index)
    {
        (*optimizations)[index] = OptimizationParams();
    }

    testScheme.reserve(elementCount + numMaxElementCountInReplacements);
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

uint PostProcessor::findInversedElementsSequence(const OptScheme& scheme, uint startPosition)
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
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    OptScheme optimizedScheme = scheme;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectForMergeOptimization,
            swapElementsWithMerge, &repeat, false, false);

        *optimized = *optimized || repeat;
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::reduceConnectionsOptimization( OptScheme& scheme, bool* optimized)
{
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    OptScheme optimizedScheme = scheme;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectForReduceConnectionsOptimization,
            swapElementsWithConnectionReduction, &repeat, false, false);

        *optimized = *optimized || repeat;
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::transferOptimization(OptScheme& scheme, bool* optimized)
{
    assert(optimized, string("Null 'optimized' pointer"));
    *optimized = false;

    OptScheme optimizedScheme = scheme;
    bool repeat = true;

    while(repeat)
    {
        optimizedScheme = tryOptimizationTactics(optimizedScheme, selectForTransferOptimization,
            swapElementsWithTransferOptimization, &repeat, false, true);

        *optimized = *optimized || repeat;
    }

    return optimizedScheme;
}

PostProcessor::OptScheme PostProcessor::getFullScheme(const OptScheme& scheme, bool heavyRight /*= true*/)
{
    OptScheme fullScheme;
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
    bool lessComplexityRequired, int* startIndex /* = 0 */)
{
    Optimizations optimizations;
    OptScheme optimizedScheme;

    bool schemeOptimized = false;
    int elementCount = scheme.size();

    prepareSchemeForOptimization(scheme, &optimizations);

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
                int leftTransferedIndex  = leftIndex;
                int rightTransferedIndex = rightIndex;

                // transfer elements if needed
                if(leftTransferedIndex + 1 != rightTransferedIndex)
                {
                    // transfer right element to left at maximum
                    rightTransferedIndex = getMaximumTransferIndex(scheme, rightElement, rightIndex, leftIndex);

                    if(rightTransferedIndex != leftIndex + 1)
                    {
                        // transfer left element to left at maximum (just once)
                        if(leftElementMaxTransferIndex == -1)
                        {
                            leftElementMaxTransferIndex = getMaximumTransferIndex(scheme, leftElement,
                                leftIndex, elementCount - 1);
                        }

                        leftTransferedIndex = leftElementMaxTransferIndex;
                    }

                    // compare indices
                    if(leftTransferedIndex + 1 >= rightTransferedIndex)
                    {
                        // good right element, try to apply optimization
                        leftTransferedIndex = rightTransferedIndex - 1; // keep left element maximum left aligned
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
                    insertReplacements(scheme, &testScheme, leftIndex, leftTransferedIndex,
                        rightIndex, rightTransferedIndex, leftReplacement, rightReplacement);

                    optimizedScheme = removeDuplicates(testScheme);
                    int optimizedSchemeSize = optimizedScheme.size();

                    if(lessComplexityRequired)
                    {
                        if(optimizedSchemeSize == elementCount
                            && secondPassOptimizationFlag)
                        {
                            secondPassOptimizationFlag = false;
                            complexityDelta = optimizedSchemeSize - elementCount;

                            bool tempFlag = false;
                            optimizedScheme = transferOptimization(optimizedScheme, &tempFlag);

                            complexityDelta = 0;
                            secondPassOptimizationFlag = false;
                        }

                        int newElementCount = optimizedScheme.size();
                        schemeOptimized = (newElementCount + complexityDelta < elementCount);
                    }
                    else
                    {
                        schemeOptimized = true;
                    }
                }
                else
                {
                    // just remove left and right elements from scheme
                    OptimizationParams& leftOptimization = optimizations[leftIndex];
                    leftOptimization.remove = true;

                    OptimizationParams& rightOptimization = optimizations[rightIndex];
                    rightOptimization.remove = true;

                    schemeOptimized = true;
                    optimizedScheme = applyOptimizations(scheme, optimizations);
                }
            }
        }
    }

    if(optimizationSucceeded)
    {
        *optimizationSucceeded = schemeOptimized;
    }

    if(!schemeOptimized)
    {
        optimizedScheme = scheme;
    }

    return optimizedScheme;
}

int PostProcessor::getMaximumTransferIndex(const OptScheme& scheme,
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

void PostProcessor::insertReplacements(const OptScheme& originalScheme,
    OptScheme* resultScheme,
    int leftIndex, int leftTransferedIndex,
    int rightIndex, int rightTransferedIndex,
    const list<ReverseElement>& leftReplacement,
    const list<ReverseElement>& rightReplacement )
{
    // check input parameters
    assert(resultScheme, string("PostProcessor: resultScheme == 0"));

    assert(leftIndex <= leftTransferedIndex,
        string("PostProcessor: wrong left index for replacements insertion"));

    assert(rightIndex >= rightTransferedIndex,
        string("PostProcessor: wrong right index for replacements insertion"));

    assert(leftTransferedIndex + 1 == rightTransferedIndex,
        string("PostProcessor: leftTransferedIndex + 1 != rightTransferedIndex"))

    int elementCount = originalScheme.size();
    uint leftReplacementSize  =  leftReplacement.size();
    uint rightReplacementSize = rightReplacement.size();

    // check element count in left and right replacement
    assert(leftReplacementSize + rightReplacementSize <= numMaxElementCountInReplacements,
        string("PostProcessor: too many elements in replacements"));

    resultScheme->reserve(elementCount + numMaxElementCountInReplacements);
    resultScheme->resize(0);

    // insert all elements until leftTransferedIndex
    for(int index = 0; index <= leftTransferedIndex; ++index)
    {
        if(index == leftIndex)
        {
            continue;
        }

        const ReverseElement& element = originalScheme[index];
        resultScheme->push_back(element);
    }

    // insert replacements
    resultScheme->insert(resultScheme->end(),  leftReplacement.cbegin(),  leftReplacement.cend());
    resultScheme->insert(resultScheme->end(), rightReplacement.cbegin(), rightReplacement.cend());

    // insert rest elements
    for(int index = rightTransferedIndex; index < elementCount; ++index)
    {
        if(index == rightIndex)
        {
            continue;
        }

        const ReverseElement& element = originalScheme[index];
        resultScheme->push_back(element);
    }
}

}   // namespace ReversibleLogic
