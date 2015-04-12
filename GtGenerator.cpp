#include "std.hpp"

namespace ReversibleLogic
{

Scheme GtGenerator::generate(const TruthTable& table, ostream& outputLog)
{
    float totalTime = 0;
    float time = 0;

    n = 0;
    {
        AutoTimer timer(&time);

        checkPermutationValidity(table);
        tie(n, permutation) = getPermutation(table);
    }

    // log permutation creation parameters
    outputLog << "Permutation creation time: ";
    outputLog << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
    outputLog << " sec" << endl;

    totalTime += time;

    debugLog("GtGenerator::generate()-dump-permutation", [&](ostream& out)->void
    {
        out << permutation << endl;
    });

    Scheme scheme;
    time = 0;

    if (permutation.length())
    {
        AutoTimer timer(&time);

        Scheme::iterator targetIter = scheme.end();

        shared_ptr<PartialGtGenerator> partialGenerator(new PartialGtGenerator());
        partialGenerator->setPermutation(permutation, n);
        partialGenerator->prepareForGeneration();

        while (partialGenerator)
            partialGenerator = reducePermutation(partialGenerator, n, &scheme, &targetIter);
    }

    // log scheme synthesis parameters
    outputLog << "Scheme synthesis time: ";
    outputLog << setiosflags(ios::fixed) << setprecision(5) << time;
    outputLog << " ms" << endl;
    outputLog << "Complexity before optimization: " << scheme.size() << endl;

    totalTime += time;
    time = 0;

    {
        AutoTimer timer(&time);
        scheme = PostProcessor().optimize(scheme);
    }

    bool isValid = TruthTableUtils::checkSchemeAgainstPermutationVector(scheme, table);
    assert(isValid, string("Generated scheme is not valid"));

    // log post processing parameters
    outputLog << "Optimization time: ";
    outputLog << setiosflags(ios::fixed) << setprecision(5) << time;
    outputLog << " ms" << endl;
    outputLog << "Complexity after optimization: " << scheme.size() << endl;

    totalTime += time;

    outputLog << "Total time: ";
    outputLog << setiosflags(ios::fixed) << setprecision(5) << totalTime;
    outputLog << " ms" << endl;

    return scheme;
}

shared_ptr<PartialGtGenerator> GtGenerator::reducePermutation(shared_ptr<PartialGtGenerator> partialGenerator,
    uint n, Scheme* scheme, Scheme::iterator* targetIter)
{
    debugLog("GtGenerator::reducePermutation()-dump-transposition-count", [&](ostream& out)->void
    {
        const Permutation& perm = partialGenerator->getPermutation();
        uint transpCount = 0;
        static uint stepCount = 0;

        for (auto cycle : perm)
        {
            uint elementCount = cycle->length();
            transpCount += elementCount - 1;
        }

        out << "Step " << ++stepCount << ", transposition count = " << transpCount << '\n';
    });

    shared_ptr<PartialGtGenerator> restGenerator = 0;

    bool isLeftAndRightMultiplicationDiffers = partialGenerator->isLeftAndRightMultiplicationDiffers();
    if(isLeftAndRightMultiplicationDiffers)
    {
        // get left choice
        Permutation leftMultipliedPermutation  = partialGenerator->getResidualPermutation(true);
        shared_ptr<PartialGtGenerator> leftGenerator(new PartialGtGenerator());

        leftGenerator->setPermutation(leftMultipliedPermutation, n);
        leftGenerator->prepareForGeneration();

        // get right choice
        Permutation rightMultipliedPermutation = partialGenerator->getResidualPermutation(false);
        shared_ptr<PartialGtGenerator> rightGenerator(new PartialGtGenerator());

        rightGenerator->setPermutation(rightMultipliedPermutation, n);
        rightGenerator->prepareForGeneration();

        debugLog("GtGenerator::reducePermutation()-dump-left-right", [&](ostream& out)->void
        {
            out << "============================\n";
            out << "Left:\n" << leftMultipliedPermutation << '\n';
            out << "\nRight:\n" << rightMultipliedPermutation << '\n' << endl;
        });

        // compare left and right choices and choose the best
        PartialResultParams leftPartialResultParams  =  leftGenerator->getPartialResultParams();
        PartialResultParams rightPartialResultParams = rightGenerator->getPartialResultParams();

        bool isLeftBetter = leftPartialResultParams.isBetterThan(rightPartialResultParams);

        debugBehavior("GtGenerator::reducePermutation()-right-always-better", [&]()->void
        {
            isLeftBetter = false;
        });
        
        if(isLeftBetter)
        {
            debugLog("GtGenerator::reducePermutation()-dump-left", [&](ostream& out)->void
            {
                out << "Left:\n" << leftMultipliedPermutation << endl;
            });

            implementPartialResult(*partialGenerator, true, scheme, targetIter);
            restGenerator = leftGenerator;
        }
        else
        {
            debugLog("GtGenerator::reducePermutation()-dump-right", [&](ostream& out)->void
            {
                out << "Right:\n" << rightMultipliedPermutation << endl;
            });

            implementPartialResult(*partialGenerator, false, scheme, targetIter);
            restGenerator = rightGenerator;
        }
    }
    else
    {
        implementPartialResult(*partialGenerator, true, scheme, targetIter);

        // get residual permutation and iterate on it
        Permutation residualPermutation = partialGenerator->getResidualPermutation(true);
        if(!residualPermutation.isEmpty())
        {
            debugLog("GtGenerator::reducePermutation()-dump-residual", [&](ostream& out)->void
            {
                out << "Residual:\n" << residualPermutation << endl;
            });

            restGenerator = shared_ptr<PartialGtGenerator>(new PartialGtGenerator());
            restGenerator->setPermutation(residualPermutation, n);
            restGenerator->prepareForGeneration();
        }
    }

    return restGenerator;
}

void GtGenerator::implementPartialResult(PartialGtGenerator& partialGenerator,
    bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter)
{
    deque<ReverseElement> elements = partialGenerator.implementPartialResult();
    assertd(elements.size(), string("GtGenerator: partial result is empty"));

    Scheme::iterator localIterator = *targetIter;
    forrcin(element, elements)
        localIterator = scheme->insert(localIterator, *element);

    if(isLeftMultiplication)
        advance(localIterator, elements.size());

    *targetIter = localIterator;
}

void GtGenerator::checkPermutationValidity(const TruthTable& table)
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

tuple<uint, Permutation> GtGenerator::getPermutation(const TruthTable& table)
{
    Permutation permutation = PermutationUtils::createPermutation(table);

    word maxValue = 0;
    for (auto cycle : permutation)
    {
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

}   // namespace ReversibleLogic
