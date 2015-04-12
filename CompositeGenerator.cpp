#include "std.hpp"

namespace ReversibleLogic
{

Scheme CompositeGenerator::generate(const TruthTable& table, ostream& outputLog)
{
    uint size = table.size();
    uint n = (uint)(log(size) / log(2));

    // todo: rewrite this part to get proper threshold
    uint threshold = n / 2;

    RmGenerator rmGenerator(threshold);
    RmGenerator::SynthesisResult rmResult;

    rmGenerator.generate(table, &rmResult);

    GtGenerator gtGenerator;
    Scheme gtScheme = gtGenerator.generate(rmResult.residualTable, outputLog);

    Scheme& scheme = rmResult.scheme;
    scheme.insert(rmResult.iter, gtScheme.cbegin(), gtScheme.cend());

    return scheme;
}

} //namespace ReversibleLogic
