#pragma once

class ProgramOptions
{
public:
    static const ProgramOptions& get();

    static void init(const Values& values);
    static void uninit();

    void load(const Values& values);

    string inputFile = "";
    string resultsFile = "results.txt";

    string schemesFolder = "schemes/";
    string schemeOutputFile = "scheme.txt";

    string tfcInputFile = "";
    string tfcOutputFile = schemeOutputFile + ".tfc";

    int transpositionsPackSize = 2;

    bool doPostOptimization = true;

    Values options;

private:
    ProgramOptions() = default;

    static shared_ptr<ProgramOptions> instance;
};