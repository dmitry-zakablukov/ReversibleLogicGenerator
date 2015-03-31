#include "std.hpp"

//static
shared_ptr<ProgramOptions> ProgramOptions::instance;

//static
const ProgramOptions& ProgramOptions::get()
{
    return *instance;
}

//static
void ProgramOptions::init(const Values& values)
{
    instance = shared_ptr<ProgramOptions>(new ProgramOptions());
    instance->load(values);
}

//static
void ProgramOptions::uninit()
{
    instance = 0;
}

void ProgramOptions::load(const Values& values)
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
