#include "std.hpp"

vector<word> getDiscreteLog(const Gf2Field& field)
{
    uint degree = field.getDegree();
    word maxElement = (word)(1 << degree);
    word elementCount = (1 << degree) - 1;

    word capacity = maxElement * maxElement;    // O(n^2)
    vector<word> table;
    table.resize(capacity);

    // init table
    for (word index = 0; index < capacity; ++index)
        table[index] = index;

    // fill vector
    for (word element = 1; element < (word)(1 << degree); ++element)
    {
        if (element != 1)
        {
            word left = element | ((uint)0 << degree);
            word right = element | ((uint)1 << degree);

            if (left != right)
                table[left] = right;
        }

        for (word n = 1; n < (word)((uint)1 << degree); ++n)
        {
            word power = field.pow(element, n);
            if (power != 1)
            {
                word left = element | (n << degree);
                word right = element | (power << degree);

                if (left != right)
                    table[left] = right;
            }
            else
                break;
        }
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
            outputFile << "Input: " << polynomial << "\n";
            outputFile << "Polynomial: " << polynomialToString(polynomial) << "\n";

            Gf2Field field(polynomial);
            vector<word> table = getDiscreteLog(field);

            Generator generator;
            auto scheme = generator.generate(table, outputFile);

            if (scheme.size() < 1000)
            {
                const char* const strSchemesFolder = "schemes/";

                if (_access(strSchemesFolder, 0))
                {
                    _mkdir(strSchemesFolder);
                }

                ostringstream schemeFileName;
                schemeFileName << strSchemesFolder << polynomialString << ".txt";

                outputFile << "Scheme file: " << schemeFileName.str() << "\n";

                string schemeString = SchemePrinter::schemeToString(scheme, false); // vertical
                //string schemeString = SchemePrinter::schemeToString(scheme, true); // horizontal

                ofstream schemeFile(schemeFileName.str());
                schemeFile << schemeString;
                schemeFile.close();
            }

            outputFile << "\n===============================================================\n";
            outputFile.flush();
        }
        catch (exception& ex)
        {
            outputFile << ex.what() << '\n';
        }
        catch (...)
        {
            outputFile << "Unknown exception\n";
        }
    }

    inputFile.close();
    outputFile.close();
}
