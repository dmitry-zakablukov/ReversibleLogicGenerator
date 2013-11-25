#include "std.hpp"

using namespace ReversibleLogic;

Generator::Scheme getRd53_8of12_goodPart()
{
    Generator::Scheme scheme;
    uint n = 7;

    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(2, 3, 4, END), mask(3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(2, 3, 4, END), mask(3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(0, 2, 3, 4, END), mask(4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 2, 3, 4, END), mask(4, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 2, 3, 4, END), mask(4, END)) );
    scheme.push_back( ReverseElement(n, mask(1, END), mask(0, 2, 3, 4, END), mask(3, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 2, 3, 4, END), mask(3, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 2, 3, 4, END), mask(3, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, 3, 4, END), mask(3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, 2, 4, END), mask(0, 2, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, 2, 3, 4, END), mask(0, 2, 3, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, 2, 3, 4, END), mask(3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, 2, 3, 4, END), mask(0, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 1, 2, 3, 4, END), mask(0, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(5, END), mask(0, 1, 2, 3, 4, END)) );
    scheme.push_back( ReverseElement(n, mask(6, END), mask(0, 1, 2, 3, 4, END)) );

    return scheme;
}

void testOptimization( int argc, const char* argv[] )
{
    const char strDefaultOutputFileName[] = "results.txt";

    ofstream outputFile;
    if(argc > 1)
    {
        outputFile.open(argv[1]);
    }
    else
    {
        outputFile.open(strDefaultOutputFileName);
    }

    try
    {
        Generator::Scheme scheme = getRd53_8of12_goodPart();
        PostProcessor optimizer;

        uint elementCount = scheme.size();

        outputFile << "Complexity before optimization: " << scheme.size() << '\n';

        vector<ReverseElement> optimizedScheme(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            optimizedScheme[index] = scheme[index];
        }

        //optimizedScheme = optimizer.optimize(optimizedScheme);

        elementCount = optimizedScheme.size();
        scheme.resize(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            scheme[index] = optimizedScheme[index];
        }

        outputFile << "Complexity after optimization: " << scheme.size() << '\n';

        string schemeString = SchemePrinter::schemeToString(scheme, false);

        outputFile << schemeString;

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

    outputFile.close();
}
