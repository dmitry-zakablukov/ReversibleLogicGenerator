#include "std.hpp"

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

word rotate(word x, word shift, word n)
{
    word mask = ((word)1 << n) - 1;

    word y = (x << shift) & mask;
    y |= (x >> (n - shift)) & mask;

    return y;
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

TruthTable getHwb(uint n)
{
    word count = 1 << n;
    word mask = count - 1;
    
    TruthTable table;
    table.resize(count);

    for (word x = 0; x < count; ++x)
    {
        uint weight = countNonZeroBits(x);

        word y = ((x << weight) & mask) |
            ((x >> (n - weight)) & mask);

        table[x] = y;
    }

    return table;
}

vector<word> getLinearWithMemory()
{
    using namespace ReversibleLogic;

    uint n = 10;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, 1 << 9, 1 << 0) );
    scheme.push_back( ReverseElement(n, 1 << 9, 1 << 4) );
    scheme.push_back( ReverseElement(n, 1 << 0, 1 << 1) );
    scheme.push_back( ReverseElement(n, 1 << 1, 1 << 2) );
    scheme.push_back( ReverseElement(n, 1 << 2, 1 << 3) );
    scheme.push_back( ReverseElement(n, 1 << 3, 1 << 4) );
    scheme.push_back( ReverseElement(n, 1 << 4, 1 << 5) );
    scheme.push_back( ReverseElement(n, 1 << 5, 1 << 6) );
    scheme.push_back( ReverseElement(n, 1 << 6, 1 << 7) );
    scheme.push_back( ReverseElement(n, 1 << 7, 1 << 8) );
    scheme.push_back( ReverseElement(n, 1 << 8, 1 << 9) );

    vector<word> table = makePermutationFromScheme(scheme, n);
    return table;
}

vector<word> getLinearMemoryless()
{
    using namespace ReversibleLogic;

    uint n = 9;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, 1 << 0, 1 << 1) );
    scheme.push_back( ReverseElement(n, 1 << 1, 1 << 2) );
    scheme.push_back( ReverseElement(n, 1 << 2, 1 << 3) );
    scheme.push_back( ReverseElement(n, 1 << 3, 1 << 4) );
    scheme.push_back( ReverseElement(n, 1 << 4, 1 << 5) );
    scheme.push_back( ReverseElement(n, 1 << 5, 1 << 6) );
    scheme.push_back( ReverseElement(n, 1 << 6, 1 << 7) );
    scheme.push_back( ReverseElement(n, 1 << 7, 1 << 8) );
    scheme.push_back( ReverseElement(n, 1 << 8, 1 << 0) );
    scheme.push_back( ReverseElement(n, 1 << 8, 1 << 1) );
    scheme.push_back( ReverseElement(n, 1 << 8, 1 << 2) );
    scheme.push_back( ReverseElement(n, 1 << 8, 1 << 3) );

    vector<word> table = makePermutationFromScheme(scheme, n);
    return table;
}

vector<word> getRd53()
{
    using namespace ReversibleLogic;

    uint n = 7;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 1, 2, 3, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 1, 2, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(1, 2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 1, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, END)) );    
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 3, END)) );    
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(4, END), mask(3, END)) );

    vector<word> table = makePermutationFromScheme(scheme, n);
    return table;
}


vector<word> getSimple()
{
    using namespace ReversibleLogic;

    uint n = 4;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, 1, 14, 8) );
    scheme.push_back( ReverseElement(n, 2, 13, 8) );
    scheme.push_back( ReverseElement(n, 1, 14, 8) );
    scheme.push_back( ReverseElement(n, 4, 11) );
    scheme.push_back( ReverseElement(n, 8, 7) );
    scheme.push_back( ReverseElement(n, 4, 11) );

    ///scheme.push_back( ReverseElement(n, 1 << 1, 1 << 0) );
    ///scheme.push_back( ReverseElement(n, 1 << 2, 1 << 1) );
    ///scheme.push_back( ReverseElement(n, 1 << 3, 1 << 2) );

    //scheme.push_back( ReverseElement(n, 1 << 1, 1 << 0) );
    //scheme.push_back( ReverseElement(n, 1 << 3, 1 << 2) );

    ////n = 6;
    ////scheme.push_back( ReverseElement(n, 1 << 1, 1 << 0) );
    ////scheme.push_back( ReverseElement(n, 1 << 3, 1 << 2) );
    ////scheme.push_back( ReverseElement(n, 1 << 5, 1 << 4) );

    vector<word> table = makePermutationFromScheme(scheme, n);
    return table;
}

vector<word> getBadCase()
{
    using namespace ReversibleLogic;

    uint n = 4;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, mask(3, END), mask(1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, END)) );

    vector<word> table = makePermutationFromScheme(scheme, n);
    return table;
}

void testSynthesis( int argc, const char* argv[] )
{
    using namespace ReversibleLogic;

    const char strDefaultInputFileName[]  = "irreducible_polynomials.txt";
    const char strDefaultOutputFileName[] = "results.txt";

    ifstream inputFile;
    if(argc > 1)
    {
        inputFile.open(argv[1]);
    }
    else
    {
        inputFile.open(strDefaultInputFileName);
    }

    ofstream outputFile;
    if(argc > 2)
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

            TruthTable table;
            table = getHwb(9);

            //typedef struct 
            //{
            //    DegreeChooseFunc func;
            //    const char* desc;
            //} TaskInfo;
            //
            //const uint numTaskCount = 8;
            //TaskInfo tasks[numTaskCount] =
            //{
            //    { chooseMinDegree, "Minimum degree:" },
            //    { chooseMaxDegree, "Maximum degree:" },
            //    { chooseDegreeWithMinDistance, "Degree with minimum distance:" },
            //    { chooseRandomDegree, "Random degree:" },
            //    { chooseRandomDegree, "Random degree:" },
            //    { chooseRandomDegree, "Random degree:" },
            //    { chooseRandomDegree, "Random degree:" },
            //    { chooseRandomDegree, "Random degree:" },
            //};
            //
            //srand((uint)time(0));
            //
            //for (uint taskIndex = 0; taskIndex < 2 /*numTaskCount*/; ++taskIndex)
            {
                //const TaskInfo& task = tasks[taskIndex];
                //
                //outputFile << task.desc << endl;
                //
                //table = getDiscreteLogWithPrimitiveElement(field, task.func);

                Generator generator;
                auto scheme = generator.generate(table, outputFile);

                //GeneratorWithMemory generator;
                //auto scheme = generator.generateFast(table, outputFile);

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
