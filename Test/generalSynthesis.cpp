#include "std.hpp"

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

TruthTable getLinearWithMemory()
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

    TruthTable table = makePermutationFromScheme(scheme, n);
    return table;
}

TruthTable getLinearMemoryless()
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

    TruthTable table = makePermutationFromScheme(scheme, n);
    return table;
}

TruthTable getRd53()
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

    TruthTable table = makePermutationFromScheme(scheme, n);
    return table;
}

TruthTable getSimple()
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

    TruthTable table = makePermutationFromScheme(scheme, n);
    return table;
}

TruthTable getBadCase()
{
    using namespace ReversibleLogic;

    uint n = 4;
    Scheme scheme;

    scheme.push_back( ReverseElement(n, mask(3, END), mask(1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, END)) );

    TruthTable table = makePermutationFromScheme(scheme, n);
    return table;
}

void generalSynthesis(int argc, const char* argv[])
{
    using namespace ReversibleLogic;

    const char strDefaultOutputFileName[] = "results.txt";
    string resultsFileName;

    if(argc > 1)
    {
        resultsFileName = argv[1];
    }
    else
    {
        resultsFileName = strDefaultOutputFileName;
    }

    ofstream outputFile(resultsFileName);

    try
    {
        TruthTable table;
        table = getHwb(11);
        //table = getRd53();

        Generator generator;
        auto scheme = generator.generate(table, outputFile);

        {
            const char* const strSchemesFolder = "schemes/";

            if (_access(strSchemesFolder, 0))
            {
                _mkdir(strSchemesFolder);
            }

            ostringstream schemeFileName;
            schemeFileName << strSchemesFolder << "result_scheme.txt";

            outputFile << "Scheme file: " << schemeFileName.str() << endl;

            //string schemeString = SchemePrinter::schemeToString(scheme, false); // vertical
            string schemeString = SchemePrinter::schemeToString(scheme, true); // horizontal

            ofstream schemeFile(schemeFileName.str());
            if (scheme.size() < 1000)
                schemeFile << schemeString;
            else
                schemeFile << "Scheme too large" << endl;
            schemeFile.close();

            ofstream tfcFile(schemeFileName.str() + ".tfc");
            TfcFormatter::format(tfcFile, scheme);
            tfcFile.close();
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
    outputFile.close();
}
