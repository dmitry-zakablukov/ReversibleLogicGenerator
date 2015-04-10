#include "std.hpp"

namespace ReversibleLogic
{

Scheme RmGenerator::generate(const TruthTable& table, ostream& outputLog)
{
    RmSpectra spectra = RmSpectraUtils::calculateRmSpectra(table);

    Scheme scheme;
    return scheme;
}

} //namespace ReversibleLogic