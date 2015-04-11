#include "std.hpp"

namespace ReversibleLogic
{

Scheme CompositeGenerator::generate(const TruthTable& table, ostream& outputLog)
{
    // todo: use both GtGenerator and RmGenerator
    RmGenerator rmGenerator;
    GtGenerator gtGenerator;

    Scheme scheme = rmGenerator.generate(table, outputLog);
    return scheme;
}

} //namespace ReversibleLogic
