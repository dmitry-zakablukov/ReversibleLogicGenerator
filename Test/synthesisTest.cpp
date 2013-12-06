#include "std.hpp"

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
    
    //scheme.push_back( ReverseElement(n, 1 << 5, (1 << 0) + (1 << 4)) );
    //scheme.push_back( ReverseElement(n, 1 << 3, 1 << 0) );
    //scheme.push_back( ReverseElement(n, 1 << 5, (1 << 3) + (1 << 4)) );
    //scheme.push_back( ReverseElement(n, 1 << 4, 1 << 3) );

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

    try
    {
        //outputFile << "Input: " << polynomial << "\n";
        //outputFile << "Polynomial: " << polynomialToString(polynomial) << "\n";

        //Gf2Field field(polynomial);
        //vector<word> table = getDiscreteLog(field);

        vector<word> table;
        //table = getLinearMemoryless();
        table = getRd53();
        //table = getBadCase();
        //table = getSimple();

        Generator generator;
        auto scheme = generator.generate(table, outputFile);

        if(scheme.size() < 1000)
        {
            const char* const strSchemesFolder = "schemes/";

            if(access(strSchemesFolder, 0))
            {
                mkdir(strSchemesFolder);
            }

            //ostringstream schemeFileName;
            //schemeFileName << strSchemesFolder << polynomialString << ".txt";

            //outputFile << "Scheme file: " << schemeFileName.str() << "\n";

            string schemeString = SchemePrinter::schemeToString(scheme, false); // vertical
            //string schemeString = SchemePrinter::schemeToString(scheme, true); // horizontal

            //ofstream schemeFile(schemeFileName.str());
            //schemeFile << schemeString;
            //schemeFile.close();

            outputFile << schemeString;
        }

        outputFile << "\n===============================================================\n";
        outputFile.flush();
    }
    catch(exception& ex)
    {
        outputFile << ex.what() << '\n';
    }
    catch(...)
    {
        outputFile << "Unknown exception\n";
    }

    inputFile.close();
    outputFile.close();
}
