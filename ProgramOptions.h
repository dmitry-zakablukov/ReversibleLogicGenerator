#pragma once

class ProgramOptions
{
public:
    virtual ~ProgramOptions() = default;

    static const ProgramOptions& get();

    static void init(const Values& values);
    static void uninit();

    void load(const Values& values);

    string inputFile = "";
    string resultsFile = "results.txt";

    string schemesFolder = "schemes/";
    string schemeOutputFile = "scheme.txt";

    int rmGeneratorWeightThreshold = -1;
    int transpositionsPackSize = 2;

    bool isDebugBehaviorEnabled = false;

    bool doPostOptimization = true;
    uint maxElementsDistanceForOptimization = 20;
    uint maxSubSchemeSizeForOptimization = 100;

    bool isTuningEnabled = false;

    Values options;

private:
    ProgramOptions() = default;

    static shared_ptr<ProgramOptions> instance;
};