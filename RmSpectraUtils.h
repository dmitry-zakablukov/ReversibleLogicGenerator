#pragma once

namespace ReversibleLogic
{

typedef vector<word> RmSpectra;

class RmSpectraUtils
{
public:
    static RmSpectra calculateRmSpectra(const TruthTable& table);
};

} //namespace ReversibleLogic