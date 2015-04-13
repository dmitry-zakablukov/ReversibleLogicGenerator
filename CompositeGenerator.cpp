#include "std.hpp"

namespace ReversibleLogic
{

Scheme CompositeGenerator::generate(const TruthTable& table, ostream& outputLog)
{
    float totalTime = 0;
    float time = 0;

    // process truth table with Reed-Muller generator
    uint size = table.size();
    uint n = (uint)(log(size) / log(2));

    uint threshold = getRmGeneratorWeightThreshold(n);
    outputLog << "RM generator index weight threshold: " << threshold << endl;

    RmGenerator rmGenerator(threshold);
    RmGenerator::SynthesisResult rmResult;

    {
        AutoTimer timer(&time);
        rmGenerator.generate(table, &rmResult);
    }
    totalTime += time;

    outputLog << "RM generator time: ";
    logTime(outputLog, time);
    outputLog << " ms\nRM scheme complexity: " << rmResult.scheme.size() << endl;

    // process residual truth table with Group Theory based generator
    GtGenerator gtGenerator;
    Scheme gtScheme;

    {
        AutoTimer timer(&time);
        gtScheme = gtGenerator.generate(rmResult.residualTable);
    }
    totalTime += time;

    outputLog << "GT generator time: ";
    logTime(outputLog, time);
    outputLog << " ms\nGT scheme complexity: " << gtScheme.size() << endl;

    // combine GT and RM schemes
    Scheme& scheme = rmResult.scheme;
    scheme.insert(scheme.end(), gtScheme.cbegin(), gtScheme.cend());

    outputLog << "Complexity before optimization: " << scheme.size() << endl;

    // optimize scheme complexity
    PostProcessor postProcessor;

    {
        AutoTimer timer(&time);
        scheme = postProcessor.optimize(scheme);
    }
    totalTime += time;

    bool isValid = TruthTableUtils::checkSchemeAgainstPermutationVector(scheme, table);
    assert(isValid, string("Generated scheme is not valid"));

    // log post processing parameters
    outputLog << "Optimization time: ";
    logTime(outputLog, time);
    outputLog << " ms\nComplexity after optimization: " << scheme.size() << endl;

    outputLog << "Total time: ";
    logTime(outputLog, totalTime);
    outputLog << " ms" << endl;

    return scheme;
}

void CompositeGenerator::logTime(ostream& out, float time)
{
    out << setiosflags(ios::fixed) << setprecision(2) << time / 1000;
}

uint CompositeGenerator::getRmGeneratorWeightThreshold(uint n)
{
    uint threshold = (uint)ProgramOptions::get().rmGeneratorWeightThreshold;
    if (threshold == uintUndefined)
    {
        // todo: choose auto value
        threshold = n;
    }

    return threshold;
}

} //namespace ReversibleLogic
