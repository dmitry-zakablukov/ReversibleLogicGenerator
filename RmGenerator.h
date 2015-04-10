#pragma once

namespace ReversibleLogic
{

class RmGenerator
{
public:
    Scheme generate(const TruthTable& table, ostream& outputLog);
};

} //namespace ReversibleLogic