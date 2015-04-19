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
    outputLog << "n = " << n << endl;

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
    outputLog << "RM scheme complexity: " << rmResult.scheme.size() << endl;

    // process residual truth table with Group Theory based generator
    GtGenerator gtGenerator;

    Scheme gtLeftScheme;
    Scheme gtRightScheme;

    {
        AutoTimer timer(&time);
        gtLeftScheme = gtGenerator.generate(rmResult.leftMultTable);
        gtRightScheme = gtGenerator.generate(rmResult.rightMultTable);
    }
    totalTime += time;

    outputLog << "GT generator time: ";
    logTime(outputLog, time);
    outputLog << "GT left scheme complexity: " << gtLeftScheme.size() << endl;
    outputLog << "GT right scheme complexity: " << gtRightScheme.size() << endl;

    // combine GT and RM schemes
    Scheme& scheme = rmResult.scheme;

    RmGenerator::PushPolicy pushPolicy = rmGenerator.getPushPolicy();
    if (pushPolicy.defaultPolicy)
    {
        if (gtLeftScheme.size() < gtRightScheme.size())
            scheme.insert(scheme.begin(), gtLeftScheme.cbegin(), gtLeftScheme.cend());
        else
            scheme.insert(scheme.end(), gtRightScheme.cbegin(), gtRightScheme.cend());
    }
    else
    {
        scheme.insert(scheme.begin(), gtLeftScheme.cbegin(), gtLeftScheme.cend());
        scheme.insert(scheme.end(), gtRightScheme.cbegin(), gtRightScheme.cend());
    }

    outputLog << "Complexity before optimization: " << scheme.size() << endl;
    outputLog << "Quantum cost before optimization: ";
    outputLog << SchemeUtils::calculateQuantumCost(scheme) << endl;

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
    outputLog << "Complexity after optimization: " << scheme.size() << endl;
    outputLog << "Quantum cost after optimization: ";
    outputLog << SchemeUtils::calculateQuantumCost(scheme) << endl;

    outputLog << "Total time: ";
    logTime(outputLog, totalTime);

    return scheme;
}

void CompositeGenerator::logTime(ostream& out, float time)
{
    out << setiosflags(ios::fixed) << setprecision(2) << time / 1000 << " sec" << endl;
}

uint CompositeGenerator::getRmGeneratorWeightThreshold(uint n)
{
    const ProgramOptions& options = ProgramOptions::get();

    uint threshold = (uint)options.rmGeneratorWeightThreshold;
    if (threshold == uintUndefined)
    {
        uint vectorCount = 2 * options.transpositionsPackSize;
        threshold = (uint)(log(vectorCount) / log(2));

        if (threshold > n)
            threshold = n;
    }

    return threshold;
}

} //namespace ReversibleLogic
