#include "std.hpp"

namespace ReversibleLogic
{

Generator::Generator()
    : n(0)
    , permutation()
    , log(0)
{
}

Scheme Generator::generate(const PermutationTable& table, ostream& outputLog)
{
    log = &outputLog;

    float totalTime = 0;
    float time = 0;

    n = 0;
    {
        AutoTimer timer(&time);

        checkPermutationValidity(table);

        tie(n, permutation) = getPermutation(table);
    }

    *log << "Permutation creation time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    //debug
    *log << (string)permutation << "\n";

    Scheme scheme;
    Scheme::iterator targetIter = scheme.end();

    while(!permutation.isEmpty())
    {
        assert(permutation.isEven(), string("Permutation parity violation found"));

        prepareCyclesInPermutation(&permutation);
        reducePermutation(&permutation, n, &scheme, &targetIter);
    }

    //shared_ptr<PartialGenerator> partialGenerator(new PartialGenerator());
    //partialGenerator->setPermutation(permutation, n);
    //partialGenerator->prepareForGeneration();

    //while(partialGenerator)
    //{
    //    partialGenerator = reducePermutation(partialGenerator, permutation, n, &scheme, &targetIter);
    //}

    //////////////////////////////////////////////////////////////////////////

    *log << "Scheme synthesis time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    time = 0;
    {
        AutoTimer timer(&time);

        *log << "Complexity before optimization: " << scheme.size() << '\n';

        PostProcessor optimizer;

        uint elementCount = scheme.size();
        PostProcessor::OptScheme optimizedScheme(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            optimizedScheme[index] = scheme[index];
        }

        optimizedScheme = optimizer.optimize(optimizedScheme);

        elementCount = optimizedScheme.size();
        scheme.resize(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            scheme[index] = optimizedScheme[index];
        }

        *log << "Complexity after optimization: " << scheme.size() << '\n';

        bool isValid = checkSchemeAgainstPermutationVector(scheme, table);
        //debug
        assert(isValid, string("Generated scheme is not valid"));
    }

    *log << "Optimization time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    *log << " sec\n";

    totalTime += time;

    *log << "Total time: ";
    *log << setiosflags(ios::fixed) << setprecision(2) << totalTime / 1000;
    *log << " sec\n";

    //string repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    //repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    return scheme;
}

shared_ptr<PartialGenerator> Generator::reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
    const Permutation& permutation, uint n, Scheme* scheme, Scheme::iterator* targetIter)
{
    shared_ptr<PartialGenerator> restGenerator = 0;

    //bool isLeftAndRightMultiplicationDiffers = partialGenerator->isLeftAndRightMultiplicationDiffers();
    //if(isLeftAndRightMultiplicationDiffers)
    //{
    //    // get left choice
    //    Permutation leftMultipliedPermutation  = partialGenerator->getResidualPermutation(true);

    //    shared_ptr<PartialGenerator> leftGenerator(new PartialGenerator());
    //    leftGenerator->setPermutation(leftMultipliedPermutation, n, true);
    //    leftGenerator->prepareForGeneration();

    //    // get right choice
    //    Permutation rightMultipliedPermutation = partialGenerator->getResidualPermutation(false);

    //    shared_ptr<PartialGenerator> rightGenerator(new PartialGenerator());
    //    rightGenerator->setPermutation(rightMultipliedPermutation, n, false);
    //    rightGenerator->prepareForGeneration();

    //    // compare left and right choices and choose the best
    //    PartialResultParams leftPartialResultParams  =  leftGenerator->getPartialResultParams();
    //    PartialResultParams rightPartialResultParams = rightGenerator->getPartialResultParams();

    //    bool isLeftBetter = isLeftChoiceBetter(leftPartialResultParams, rightPartialResultParams);

    //    // for test purpose only
    //    // isLeftBetter = false;
    //    
    //    if(isLeftBetter)
    //    {
    //        implementPartialResult(partialGenerator, true, scheme, targetIter);
    //        restGenerator = leftGenerator;
    //    }
    //    else
    //    {
    //        implementPartialResult(partialGenerator, false, scheme, targetIter);
    //        restGenerator = rightGenerator;
    //    }
    //}
    //else
    //{
    //    implementPartialResult(partialGenerator, true, scheme, targetIter);

    //    // get residual permutation and iterate on it
    //    Permutation residualPermutation = partialGenerator->getResidualPermutation(true);

    //    if(!residualPermutation.isEmpty())
    //    {
    //        restGenerator = shared_ptr<PartialGenerator>(new PartialGenerator());
    //        restGenerator->setPermutation(residualPermutation, n, true);
    //        restGenerator->prepareForGeneration();
    //    }
    //}

    return restGenerator;
}

void Generator::reducePermutation(Permutation* permutation, uint n, Scheme* scheme, Scheme::iterator* targetIter)
{
    PartialGenerator leftGenerator;
    PartialGenerator rightGenerator;
    
    PartialGenerator* generator = 0;
    bool isLeftMultiplication = true;
    
    bool multiplicationDiffers = isLeftAndRightMultiplicationDiffers(permutation);
    if(multiplicationDiffers)
    {
        // get left choice
        leftGenerator.setPermutation(permutation, n, true);
        leftGenerator.prepareForGeneration();
        PartialResultParams leftPartialResultParams = leftGenerator.getPartialResultParams();

        // get right choice
        rightGenerator.setPermutation(permutation, n, false);
        rightGenerator.prepareForGeneration();
        PartialResultParams rightPartialResultParams = rightGenerator.getPartialResultParams();

        // compare left and right choices and choose the best
        isLeftMultiplication = isLeftChoiceBetter(leftPartialResultParams, rightPartialResultParams);

        // for test purpose only
        // isLeftBetter = false;
        generator = (isLeftMultiplication ? &leftGenerator : &rightGenerator);
    }
    else
    {
        leftGenerator.setPermutation(permutation, n, true);
        leftGenerator.prepareForGeneration();
        PartialResultParams leftPartialResultParams = leftGenerator.getPartialResultParams();

        generator = &leftGenerator;
    }

    implementPartialResult(generator, isLeftMultiplication, scheme, targetIter);
    *permutation = generator->getResidualPermutation();
}

bool Generator::isLeftChoiceBetter(const PartialResultParams& leftPartialResultParams,
    const PartialResultParams& rightPartialResultParams)
{
    bool isLeftBetter = false;

    if(leftPartialResultParams.type == rightPartialResultParams.type)
    {
        switch(leftPartialResultParams.type)
        {
        case tFullEdge:
        case tEdge:
            if(leftPartialResultParams.params.edgeCapacity == rightPartialResultParams.params.edgeCapacity)
            {
                //isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum <= rightPartialResultParams.restCyclesDistanceSum);
                isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum < rightPartialResultParams.restCyclesDistanceSum);
            }
            else
            {
                isLeftBetter = (leftPartialResultParams.params.edgeCapacity > rightPartialResultParams.params.edgeCapacity);
            }
            break;

        case tSameDiffPair:
            {
                uint leftWeight  = countNonZeroBits( leftPartialResultParams.params.diff);
                uint rightWeight = countNonZeroBits(rightPartialResultParams.params.diff);

                if(leftWeight == rightWeight)
                {
                    //isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum <= rightPartialResultParams.restCyclesDistanceSum);
                    isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum < rightPartialResultParams.restCyclesDistanceSum);
                }
                else
                {
                    isLeftBetter = (leftWeight < rightWeight);
                }
            }
            break;

        case tCommonPair:
            {
                uint leftSum = 0;

                leftSum += countNonZeroBits(leftPartialResultParams.params.common.leftDiff );
                leftSum += countNonZeroBits(leftPartialResultParams.params.common.rightDiff);
                leftSum += countNonZeroBits(leftPartialResultParams.params.common.distance );

                uint rightSum = 0;

                rightSum += countNonZeroBits(rightPartialResultParams.params.common.leftDiff );
                rightSum += countNonZeroBits(rightPartialResultParams.params.common.rightDiff);
                rightSum += countNonZeroBits(rightPartialResultParams.params.common.distance );

                if(leftSum == rightSum)
                {
                    //isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum <= rightPartialResultParams.restCyclesDistanceSum);
                    isLeftBetter = (leftPartialResultParams.restCyclesDistanceSum < rightPartialResultParams.restCyclesDistanceSum);
                }
                else
                {
                    isLeftBetter = (leftSum < rightSum);
                }
            }
            break;

        default:
            assert(false, string("Generator: can't choose on unknown partial result type"));
            break;
        }
    }
    else
    {
        isLeftBetter = leftPartialResultParams.type > rightPartialResultParams.type;
    }

    return isLeftBetter;
}

void Generator::implementPartialResult(PartialGenerator* partialGenerator,
    bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter)
{
    deque<ReverseElement> elements = partialGenerator->implementPartialResult();
    assert(elements.size(), string("Generator: partial result is empty"));

    Scheme::iterator localIterator = *targetIter;
    forrcin(element, elements)
    {
        localIterator = scheme->insert(localIterator, *element);
    }

    if(isLeftMultiplication)
    {
        advance(localIterator, elements.size());
    }

    *targetIter = localIterator;
}

void Generator::checkPermutationValidity(const PermutationTable& table)
{
    set<word> inputs;
    set<word> outputs;

    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        const word& y = table[x];

        inputs.insert(x);
        outputs.insert(y);
    }

    assert(inputs.size() == table.size(),
           string("Number of inputs in permutation table is too small"));

    assert(outputs.size() == table.size(),
           string("Number of outputs in permutation table is too small"));
}

tuple<uint, Permutation> Generator::getPermutation(const PermutationTable& table)
{
    Permutation permutation = PermutationUtils::createPermutation(table);

    word maxValue = 0;
    forin(cycleIter, permutation)
    {
        const shared_ptr<Cycle>& cycle = *cycleIter;
        uint elementCount = cycle->length();
        for(uint index = 0; index < elementCount; ++index)
        {
            const word& element = (*cycle)[index];
            maxValue |= element;
        }
    }

    // find number of bits
    uint n = 0;
    word mask = 1;

    while(mask <= maxValue)
    {
        ++n;
        mask <<= 1;
    }

    return tie(n, permutation);
}

bool Generator::checkSchemeAgainstPermutationVector(const Scheme& scheme, const PermutationTable& table)
{
    bool result = true;
    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        const word& y = table[x];

        forin(iter, scheme)
        {
            const ReverseElement& element = *iter;
            x = element.getValue(x);
        }

        if(x != y)
        {
            result = false;
            break;
        }
    }

    return result;
}

void Generator::prepareCyclesInPermutation(Permutation* permutation)
{
    // prepare all cycles in permutation for disjoint
    unordered_map<word, uint> frequencyMap;
    forin(iter, *permutation)
    {
        Cycle& cycle = **iter;
        cycle.prepareForDisjoint(&frequencyMap);
    }

    // find most frequent diff
    uint maxFreq = 0;
    word bestDiff = 0;
    uint bestDiffWeight = 0;

    forcin(iter, frequencyMap)
    {
        word diff = iter->first;
        uint count = iter->second;

        if(count > maxFreq)
        {
            maxFreq = count;
            bestDiff = diff;
            bestDiffWeight = countNonZeroBits(bestDiff);
        }
        else if(maxFreq == count)
        {
            uint diffWeight = countNonZeroBits(diff);
            if(diffWeight < bestDiffWeight)
            {
                bestDiffWeight = diffWeight;
                bestDiff = diff;
            }
        }
    }

    // set best diff for all cycles in permutation
    forin(iter, *permutation)
    {
        Cycle& cycle = **iter;
        cycle.setDisjointDiff(bestDiff);
    }

    //// debug
    //cout << "Cycles in permutation prepared\n";
}

bool Generator::isLeftAndRightMultiplicationDiffers(const Permutation* permutation) const
{
    bool isDiffer = false;
    forin(iter, *permutation)
    {
        const Cycle& cycle = **iter;
        if(cycle.length() > 2)
        {
            isDiffer = true;
            break;
        }
    }

    return isDiffer;
}

}   // namespace ReversibleLogic
