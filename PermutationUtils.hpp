#pragma once

namespace ReversibleLogic
{

class PermutationUtils
{
public:
    typedef vector<word> Piece;

    static Permutation createPermutation(const TruthTable& inputTable);

private:
    static vector<Piece> findPieces(const TruthTable& table);
    static vector<Piece> mergePieces(const vector<Piece>& pieces);
};

}   // namespace ReversibleLogic
