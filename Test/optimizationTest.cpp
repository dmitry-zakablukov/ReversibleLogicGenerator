#include "std.hpp"

using namespace ReversibleLogic;

bool validateOptimizedScheme(const Scheme& before, const PostProcessor::OptScheme& after)
{
    uint n = 0;
    if (before.size())
        n = before.front().getInputCount();
    else if (after.size())
        n = after.front().getInputCount();

    word total = 1 << n;
    for (word x = 0; x < total; ++x)
    {
        word y = x;
        for (auto& element : before)
            y = element.getValue(y);

        word z = x;
        for (auto& element : after)
            z = element.getValue(z);

        if (y != z)
            return false;
    }

    return true;
}

Scheme getRd53_8of12_goodPart()
{
    Scheme scheme;
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

Scheme getDuplicatesScheme()
{
    // ( )   ( )   (+)
    // ( )( )( )( )   
    //    (+)   (+)   
    // (+)   (+)   ( )

    Scheme scheme;
    uint n = 4;

    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(3, END)) );

    return scheme;
}

Scheme getReduceConnectionsScheme()
{
    //    ( )   (0)(+)
    // ( )(0)( )( )   
    // (+)(+)   (+)( )
    //       (+)      

    Scheme scheme;
    uint n = 4;

    scheme.push_back( ReverseElement(n, mask(2, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(0, 1, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(0, 1, END), mask(0, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(2, END)) );

    return scheme;
}

Scheme getMergeScheme()
{
    //    ( )   (0)(+)
    // ( )(0)( )(0)   
    // (+)(+)   (+)( )
    //       (+)      

    Scheme scheme;
    uint n = 4;

    scheme.push_back( ReverseElement(n, mask(2, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(0, 1, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(2, END), mask(0, 1, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(2, END)) );

    return scheme;
}

Scheme getTransferScheme()
{
    // (+)( )(+) ( )(+)( )    (+)( ) ( )(+)(0)
    // (0)   (0) ( )( )( ) ( )( )( ) ( )( )( )
    // ( )( )( ) ( )   ( ) ( )   ( ) (0)(0)(0)
    //    (+)    (+)   (+) (+)   (+) (+)   (+)

    Scheme scheme;
    uint n = 4;

    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, END), mask(1, END)) );
    
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END)) );
    
    scheme.push_back( ReverseElement(n, mask(3, END), mask(1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END)) );
    
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(1, 2, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END), mask(0, 2, END)) );

    return scheme;
}

Scheme getSecondPassScheme()
{
    // (+)( )( )(+)
    //    ( )( )   
    // ( )( )   ( )
    //    (+)(+)   

    Scheme scheme;
    uint n = 4;

    scheme.push_back( ReverseElement(n, mask(0, END), mask(2, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, 2, END)) );
    scheme.push_back( ReverseElement(n, mask(3, END), mask(0, 1, END)) );
    scheme.push_back( ReverseElement(n, mask(0, END), mask(2, END)) );

    return scheme;
}

Scheme getRd53()
{
    Scheme scheme;
    uint n = 7;

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
        Scheme scheme;
        //scheme = getRd53_8of12_goodPart();
        //scheme = getDuplicatesScheme();
        //scheme = getReduceConnectionsScheme();
        //scheme = getMergeScheme();
        //scheme = getTransferScheme();
        //scheme = getSecondPassScheme();
        scheme = getRd53();
        
        PostProcessor optimizer;

        uint elementCount = scheme.size();

        outputFile << "Complexity before optimization: " << scheme.size() << "\n\n";

        string schemeString = SchemePrinter::schemeToString(scheme, true);
        outputFile << schemeString << '\n';

        PostProcessor::OptScheme optimizedScheme(elementCount);
        for(uint index = 0; index < elementCount; ++index)
        {
            optimizedScheme[index] = scheme[index];
        }

        optimizedScheme = optimizer.optimize(optimizedScheme);
        assert(validateOptimizedScheme(scheme, optimizedScheme),
            string("Optimized scheme is not valid"));

        elementCount = optimizedScheme.size();
        scheme.resize(elementCount);

        for(uint index = 0; index < elementCount; ++index)
        {
            scheme[index] = optimizedScheme[index];
        }

        outputFile << "Complexity after optimization: " << scheme.size() << "\n\n";

        schemeString = SchemePrinter::schemeToString(scheme, true);
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
