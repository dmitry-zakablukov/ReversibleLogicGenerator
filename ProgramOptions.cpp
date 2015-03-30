#include "std.hpp"

//static
ProgramOptions* ProgramOptions::instance = 0;

//static
ProgramOptions& ProgramOptions::get()
{
    if (!instance)
        instance = new ProgramOptions(); //bugbug: no delete

    return *instance;
}

void ProgramOptions::init(const Values& values)
{
    const char* strInputFile   = "input-file";
    const char* strResultsFile = "results-file";

    const char* strSchemesFolder    = "schemes-folder";
    const char* strSchemeOutputFile = "scheme-output-file";

    const char* strTfcInputFile  = "tfc-input";
    const char* strTfcOutputFile = "tfc-output";

    const char* strTranspositionsPackSize = "transpositions-pack-size";
    const char* strDoPostOptimization     = "do-post-optimization";

    // load common options from input values, default values see in header

    inputFile   = values.getString(strInputFile, inputFile);
    resultsFile = values.getString(strResultsFile, resultsFile);

    schemesFolder    = values.getString(strSchemesFolder, schemesFolder);
    schemeOutputFile = values.getString(strSchemeOutputFile, schemeOutputFile);

    tfcInputFile  = values.getString(strTfcInputFile, tfcInputFile);
    tfcOutputFile = values.getString(strTfcOutputFile, tfcOutputFile);

    transpositionsPackSize = values.getInt(strTranspositionsPackSize, transpositionsPackSize);
    doPostOptimization = values.getBool(strDoPostOptimization, doPostOptimization);

    // save input values
    options = values;
}
