#include "std.hpp"

word rotate(word x, word shift, word n)
{
    word mask = ((word)1 << n) - 1;

    word y = (x << shift) & mask;
    y |= (x >> (n - shift)) & mask;

    return y;
}

TruthTable getDiscretePowerWithPrimitiveElement(Gf2Field& field)
{
    word maxElement = (word)(1 << field.getDegree());
    word elementCount = maxElement - 1;

    word primitiveElement = field.getPrimitiveElement();
    assert(primitiveElement != wordUndefined, string("Field has no primitive elements"));

    TruthTable table;
    table.resize(maxElement);

    table[0] = 1;
    table[1] = primitiveElement;
    table[elementCount] = 0;

    // fill rest table
    word z = primitiveElement;
    for (word deg = 2; deg < elementCount; ++deg)
    {
        z = field.mul(z, primitiveElement);
        table[deg] = z;
    }

    return table;
}

word chooseMinDegree(const list<word>& degrees, const list<word>& values)
{
    word choosenDeg = wordUndefined;
    for (auto& deg : degrees)
    {
        if (deg < choosenDeg)
        {
            choosenDeg = deg;
        }
    }

    return choosenDeg;
}

word chooseMaxDegree(const list<word>& degrees, const list<word>& values)
{
    word choosenDeg = 0;
    for (auto& deg : degrees)
    {
        if (deg > choosenDeg)
        {
            choosenDeg = deg;
        }
    }

    return choosenDeg;
}

word chooseRandomDegree(const list<word>& degrees, const list<word>& values)
{
    uint count = degrees.size();
    uint offset = rand() % count;

    auto iter = degrees.cbegin();
    advance(iter, offset);

    word choosenDeg = *iter;
    return choosenDeg;
}

word chooseDegreeWithMinDistance(const list<word>& degrees, const list<word>& values)
{
    uint minDistance = uintUndefined;
    word choosenDeg = wordUndefined;

    for (auto& deg : degrees)
    {
        uint distance = 0;
        for (auto& x : values)
        {
            distance += countNonZeroBits(deg ^ x);
        }

        if (distance < minDistance)
        {
            minDistance = distance;
            choosenDeg = deg;
        }
        else if (distance == minDistance && deg < choosenDeg)
        {
            choosenDeg = deg;
        }
    }

    return choosenDeg;
}

typedef word(*DegreeChooseFunc)(const list<word>& degrees, const list<word>& values);

TruthTable getDiscreteLogWithPrimitiveElement(Gf2Field& field, DegreeChooseFunc chooseFunc = 0)
{
    word maxElement = (word)(1 << field.getDegree());
    word elementCount = maxElement - 1;

    word primitiveElement = field.getPrimitiveElement();
    assert(primitiveElement != wordUndefined, string("Field has no primitive elements"));

    TruthTable table;
    table.resize(maxElement);

    table[0] = elementCount;
    table[1] = 0;

    if (!chooseFunc)
    {
        table[primitiveElement] = 1;

        // fill rest table
        word z = primitiveElement;
        for (word deg = 2; deg < elementCount; ++deg)
        {
            z = field.mul(z, primitiveElement);
            table[z] = deg;
        }
    }
    else
    {
        unordered_set<word> visitedDegrees;
        visitedDegrees.insert(0);
        visitedDegrees.insert(elementCount);

        // fill rest table
        word z = primitiveElement;
        for (word deg = 1; deg < elementCount; ++deg)
        {
            if (visitedDegrees.find(deg) != visitedDegrees.cend())
            {
                // already processed this degree
                continue;
            }
            else
            {
                // list of degrees with rotation
                list<word> degrees{ deg };
                visitedDegrees.insert(deg);

                // list of values with squares
                word w = field.pow(z, deg);
                list<word> values{ w };

                word n = field.getDegree();
                word rotatedDeg = rotate(deg, 1, n);

                while (rotatedDeg != deg)
                {
                    visitedDegrees.insert(rotatedDeg);
                    degrees.push_back(rotatedDeg);

                    w = field.mul(w, w);
                    values.push_back(w);

                    rotatedDeg = rotate(rotatedDeg, 1, n);
                }

                word choosenDeg = chooseFunc(degrees, values);
                for (auto& x : values)
                {
                    table[x] = choosenDeg;
                }
            }
        }
    }

    return table;
}

void discreteLogSynthesis(int argc, const char* argv[])
{
    using namespace ReversibleLogic;

    const char strDefaultInputFileName[] = "irreducible_polynomials.txt";
    const char strDefaultOutputFileName[] = "results.txt";

    ifstream inputFile;
    if (argc > 1)
    {
        inputFile.open(argv[1]);
    }
    else
    {
        inputFile.open(strDefaultInputFileName);
    }

    ofstream outputFile;
    if (argc > 2)
    {
        outputFile.open(argv[2]);
    }
    else
    {
        outputFile.open(strDefaultOutputFileName);
    }

    while (inputFile.good())
    {
        string polynomialString;
        getline(inputFile, polynomialString);
        word polynomial = binStringToInt(polynomialString);

        try
        {
            Gf2Field field(polynomial);

            outputFile << "Input: " << polynomial << endl;
            outputFile << "Polynomial: " << polynomialToString(polynomial) << endl;
            outputFile << "Primitive element: " << polynomialToString(field.getPrimitiveElement()) << endl;

            if (field.getDegree() > 11)
            {
                ostringstream stream;
                stream << "Skipping field with degree " << field.getDegree() << endl;

                throw AssertionError(stream.str());
            }

            struct TaskInfo
            {
                DegreeChooseFunc func;
                const char* desc;
            };

            const uint numTaskCount = 8;
            TaskInfo tasks[numTaskCount] =
            {
                { chooseMinDegree, "Minimum degree:" },
                { chooseMaxDegree, "Maximum degree:" },
                { chooseDegreeWithMinDistance, "Degree with minimum distance:" },
                { chooseRandomDegree, "Random degree:" },
                { chooseRandomDegree, "Random degree:" },
                { chooseRandomDegree, "Random degree:" },
                { chooseRandomDegree, "Random degree:" },
                { chooseRandomDegree, "Random degree:" },
            };

            srand((uint)time(0));

            for (uint taskIndex = 0; taskIndex < numTaskCount; ++taskIndex)
            {
                const TaskInfo& task = tasks[taskIndex];

                outputFile << task.desc << endl;

                TruthTable table = getDiscreteLogWithPrimitiveElement(field, task.func);

                //Generator generator;
                //auto scheme = generator.generate(table, outputFile);

                GeneratorWithMemory generator;
                auto scheme = generator.generateFast(table, outputFile);

                if (scheme.size() < 1000)
                {
                    const char* const strSchemesFolder = "schemes/";

                    if (_access(strSchemesFolder, 0))
                    {
                        _mkdir(strSchemesFolder);
                    }

                    ostringstream schemeFileName;
                    schemeFileName << strSchemesFolder << polynomialString << ".txt";

                    outputFile << "Scheme file: " << schemeFileName.str() << endl;

                    string schemeString = SchemePrinter::schemeToString(scheme, false); // vertical
                    //string schemeString = SchemePrinter::schemeToString(scheme, true); // horizontal

                    ofstream schemeFile(schemeFileName.str());
                    schemeFile << schemeString;
                    schemeFile.close();
                }
            }
        }
        catch (exception& ex)
        {
            outputFile << ex.what() << endl;
        }
        catch (...)
        {
            outputFile << "Unknown exception" << endl;
        }

        outputFile << "\n===============================================================" << endl;
    }

    inputFile.close();
    outputFile.close();
}
