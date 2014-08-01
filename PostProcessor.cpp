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
        if (leftControlMask == rightControlMask &&
            countNonZeroBits(inversionsDiff) == 1)
        {
            return true;
        }
 
        // (0*)(01) -> (00)
        if (leftInversionMask == rightInversionMask &&
            countNonZeroBits(controlsDiff) == 1)
        {
            return true;
        }

        // (1*)(10) -> (11)
        if (controlsDiff == inversionsDiff && countNonZeroBits(controlsDiff) == 1)
        {
            return true;
        }
    }

    return false;
}

bool selectForTransferOptimization(const ReverseElement& left,
    const ReverseElement& right)
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
    assert(leftReplacement->size(), string("swapElementsWithMerge(): wrong input elements"));
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

    isNegativeControlInputsAllowed = true;
    
    //// debug: generator 4.0 - test scheme synthesis
    //return scheme;

    optimizedScheme = removeDuplicates(optimizedScheme);

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
    isNegativeControlInputsAllowed = false;

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
    assert(optimizationSucceeded, string("Null ptr 'optimizationSucceeded' (PostProcessor::tryOptimizationTactics)"));
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

        // find right element index
        for (int rightIndex = (searchPairFromEnd ? elementCount - 1 : leftIndex + 1);
            !schemeOptimized && (searchPairFromEnd ? rightIndex > leftIndex : rightIndex < elementCount);
            rightIndex += (searchPairFromEnd ? -1 : 1))
        {
            SwapResultsPair pair = getSwapResultsPair(scheme, leftIndex, rightIndex);

            uint newLeftIndex  = uintUndefined;
            uint newRightIndex = uintUndefined;
            if (isSwapResultsPairSuiteOptimizationTactics(selectionFunc, pair,
                leftIndex, rightIndex, &newLeftIndex, &newRightIndex))
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

        // stop search if not swappable
        bool withOneControlLineInverting = false;
        if (!target.isSwappable(neighborElement, &withOneControlLineInverting) || withOneControlLineInverting)
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
    assert(leftIndex < schemeSize && rightIndex < schemeSize,
        string("Wrong indices (PostProcessor::getSwapResultsPair)"));

    // TODO: check if this would be necessary
    assert(leftIndex < rightIndex, string("Unordered indices (PostProcessor::getSwapResultsPair)"));

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

    assert(toLeft.size() && toRight.size(), string("Can't merge empty swap results"));

    SwapResult left = toLeft.back();
    toLeft.pop_back();

    SwapResult right = toRight.front();
    toRight.pop_front();

    assert(left.first == right.first && left.second.end == right.second.start,
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
    assert(newLeftIndex && newRightIndex, string("Null ptr "
        "(PostProcessor::isSwapResultsPairSuiteOptimizationTactics)"));

    assert(result.forLeft.size() && result.forRight.size(),
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
    assert(scheme, string("Null ptr (PostProcessor::moveElementInScheme)"));

    uint size = scheme->size();
    assert(fromIndex < size && toIndex < size,
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

}   // namespace ReversibleLogic
