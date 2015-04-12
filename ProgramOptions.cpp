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

    const char* strRmGeneratorWeightThreshold = "rm-generator-weight-threshold";
    const char* strTranspositionsPackSize = "transpositions-pack-size";
    
    const char* strDoPostOptimization = "do-post-optimization";
    const char* strMaxElementsDistanceForOptimization = "max-elements-distance-for-optimization";
    const char* strMaxSubSchemeSizeForOptimization = "max-sub-scheme-size-for-optimization";

    const char* strEnableTuning = "enable-tuning";
    const char* strEnableDebugBehavior = "enable-debug-behavior";

    // load common options from input values, default values see in header

    inputFile   = values.getString(strInputFile, inputFile);
    resultsFile = values.getString(strResultsFile, resultsFile);

    schemesFolder    = values.getString(strSchemesFolder, schemesFolder);
    schemeOutputFile = values.getString(strSchemeOutputFile, schemeOutputFile);

    rmGeneratorWeightThreshold = values.getInt(strRmGeneratorWeightThreshold, rmGeneratorWeightThreshold);
    transpositionsPackSize = values.getInt(strTranspositionsPackSize, transpositionsPackSize);

    doPostOptimization = values.getBool(strDoPostOptimization, doPostOptimization);

    maxElementsDistanceForOptimization = values.getInt(strMaxElementsDistanceForOptimization,
        maxElementsDistanceForOptimization);

    maxSubSchemeSizeForOptimization = values.getInt(strMaxSubSchemeSizeForOptimization,
        maxSubSchemeSizeForOptimization);

    isTuningEnabled = values.getBool(strEnableTuning, isTuningEnabled);
    isDebugBehaviorEnabled = values.getBool(strEnableDebugBehavior, isDebugBehaviorEnabled);

    // save input values
    options = values;
}
