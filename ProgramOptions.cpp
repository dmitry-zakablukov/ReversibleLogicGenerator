#include "std.hpp"

//static
shared_ptr<ProgramOptions> ProgramOptions::instance;

//static
const ProgramOptions& ProgramOptions::get()
{
    assert(instance, string("ProgramOptions::get(): instance is null"));
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
    const char* strEnableDebugBehavior    = "enable-debug-behavior";
    
    const char* strDoPostOptimization = "do-post-optimization";
    const char* strMaxElementsDistanceForOptimization = "max-elements-distance-for-optimization";
    const char* strMaxSubSchemeSizeForOptimization = "max-sub-scheme-size-for-optimization";

    const char* strEnableTuning = "enable-tuning";

    // load common options from input values, default values see in header

    inputFile   = values.getString(strInputFile, inputFile);
    resultsFile = values.getString(strResultsFile, resultsFile);

    schemesFolder    = values.getString(strSchemesFolder, schemesFolder);
    schemeOutputFile = values.getString(strSchemeOutputFile, schemeOutputFile);

    tfcInputFile  = values.getString(strTfcInputFile, tfcInputFile);
    tfcOutputFile = values.getString(strTfcOutputFile, tfcOutputFile);

    transpositionsPackSize = values.getInt(strTranspositionsPackSize, transpositionsPackSize);
    isDebugBehaviorEnabled = values.getBool(strEnableDebugBehavior, isDebugBehaviorEnabled);

    doPostOptimization = values.getBool(strDoPostOptimization, doPostOptimization);

    maxElementsDistanceForOptimization = values.getInt(strMaxElementsDistanceForOptimization,
        maxElementsDistanceForOptimization);

    maxSubSchemeSizeForOptimization = values.getInt(strMaxSubSchemeSizeForOptimization,
        maxSubSchemeSizeForOptimization);

    isTuningEnabled = values.getBool(strEnableTuning, isTuningEnabled);

    // save input values
    options = values;
}
