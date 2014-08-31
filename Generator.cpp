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

    ////debug
    //*log << (string)permutation << "\n";

    Scheme scheme;

    time = 0;
    {
        const uint numIterCount = 1;
        {
            AutoTimer timer(&time);

            uint count = numIterCount;
            while (count--)
            {
                Scheme localScheme;
                Scheme::iterator targetIter = localScheme.end();

                //// debug
                //cout << "Permutation: " << (string)permutation << "\n";

                shared_ptr<PartialGenerator> partialGenerator(new PartialGenerator());
                partialGenerator->setPermutation(permutation, n);
                partialGenerator->prepareForGeneration();

                while (partialGenerator)
                {
                    partialGenerator = reducePermutation(partialGenerator, n, &localScheme, &targetIter);
                }

                scheme = localScheme;
            }
        }

        time /= numIterCount;
    }

    //////////////////////////////////////////////////////////////////////////

    *log << "Scheme synthesis time: ";
    *log << setiosflags(ios::fixed) << setprecision(5) << time;
    *log << " ms\n";
    *log << "Complexity before optimization: " << scheme.size() << '\n';

    totalTime += time;
    time = 0;
    {
        const uint numIterCount = 1;

        {
            AutoTimer timer(&time);
            uint count = numIterCount;

            while (count--)
            {
                PostProcessor optimizer;

                uint elementCount = scheme.size();
                PostProcessor::OptScheme optimizedScheme(elementCount);

                for (uint index = 0; index < elementCount; ++index)
                {
                    optimizedScheme[index] = scheme[index];
                }

                optimizedScheme = optimizer.optimize(optimizedScheme);

                elementCount = optimizedScheme.size();
                scheme.resize(elementCount);

                for (uint index = 0; index < elementCount; ++index)
                {
                    scheme[index] = optimizedScheme[index];
                }
            }
        }

        bool isValid = checkSchemeAgainstPermutationVector(scheme, table);
        //debug
        assert(isValid, string("Generated scheme is not valid"));

        time /= numIterCount;
    }

    *log << "Optimization time: ";
    *log << setiosflags(ios::fixed) << setprecision(5) << time;
    *log << " ms\n";
    *log << "Complexity after optimization: " << scheme.size() << '\n';

    totalTime += time;

    *log << "Total time: ";
    *log << setiosflags(ios::fixed) << setprecision(5) << totalTime;
    *log << " ms\n";

    //string repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    //repres = SchemePrinter::schemeToString(n, scheme, false);
    //log << repres;

    //// debug
    //cout << "Gate complexity: " << scheme.size() << '\n';

    return scheme;
}

shared_ptr<PartialGenerator> Generator::reducePermutation(shared_ptr<PartialGenerator> partialGenerator,
    uint n, Scheme* scheme, Scheme::iterator* targetIter)
{
    ////////////////////////////////////////////////////////////////////////////
    //// start of debug
    //const Permutation& perm = partialGenerator->getPermutation();
    //uint transpCount = 0;
    //static uint stepCount = 0;

    //for (auto cycle : perm)
    //{
    //    uint elementCount = cycle->length();
    //    transpCount += elementCount - 1;
    //}

    //cout << "Step " << ++stepCount << ", transposition count = " << transpCount << '\n';
    //// end of debug
    ////////////////////////////////////////////////////////////////////////////

    shared_ptr<PartialGenerator> restGenerator = 0;

    bool isLeftAndRightMultiplicationDiffers = partialGenerator->isLeftAndRightMultiplicationDiffers();
    if(isLeftAndRightMultiplicationDiffers)
    {
        // get left choice
        Permutation leftMultipliedPermutation  = partialGenerator->getResidualPermutation(true);
        shared_ptr<PartialGenerator> leftGenerator(new PartialGenerator());

        leftGenerator->setPermutation(leftMultipliedPermutation, n);
        leftGenerator->prepareForGeneration();

        // get right choice
        Permutation rightMultipliedPermutation = partialGenerator->getResidualPermutation(false);
        shared_ptr<PartialGenerator> rightGenerator(new PartialGenerator());

        rightGenerator->setPermutation(rightMultipliedPermutation, n);
        rightGenerator->prepareForGeneration();

        //// debug
        //cout << "============================\n";
        //cout << "Left:\n" << (string)leftMultipliedPermutation << '\n';
        //cout << "\nRight:\n" << (string)rightMultipliedPermutation << '\n\n';

        // compare left and right choices and choose the best
        PartialResultParams leftPartialResultParams  =  leftGenerator->getPartialResultParams();
        PartialResultParams rightPartialResultParams = rightGenerator->getPartialResultParams();

        bool isLeftBetter = leftPartialResultParams.isBetterThan(rightPartialResultParams);

        // for test purpose only
        // isLeftBetter = false;
        
        if(isLeftBetter)
        {
            //// debug
            //cout << "Left:\n" << (string)leftMultipliedPermutation << '\n';

            implementPartialResult(*partialGenerator, true, scheme, targetIter);
            restGenerator = leftGenerator;
        }
        else
        {
            //// debug
            //cout << "Right:\n" << (string)rightMultipliedPermutation << '\n';

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
            //// debug
            //cout << "Residual:\n" << (string)residualPermutation << '\n';

            restGenerator = shared_ptr<PartialGenerator>(new PartialGenerator());
            restGenerator->setPermutation(residualPermutation, n);
            restGenerator->prepareForGeneration();
        }
    }

    return restGenerator;
}

void Generator::implementPartialResult(PartialGenerator& partialGenerator,
    bool isLeftMultiplication, Scheme* scheme, Scheme::iterator* targetIter)
{
    deque<ReverseElement> elements = partialGenerator.implementPartialResult();
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

bool Generator::checkSchemeAgainstPermutationVector(const Scheme& scheme, const PermutationTable& table)
{
    bool result = true;
    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        const word& y = table[x];

        for (auto& element : scheme)
        {
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

}   // namespace ReversibleLogic
