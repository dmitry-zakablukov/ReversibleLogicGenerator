#pragma once

namespace ReversibleLogic
{

class PermutationUtils
{
public:
    typedef vector<word> Piece;

    static Permutation createPermutation(const TruthTable& inputTable);

private:
    // Applies various optimization techniques to the table
    static TruthTable expandTable(const TruthTable& table);

    static vector<Piece> findPieces(const TruthTable& table);
    static vector<Piece> mergePieces(const vector<Piece>& pieces);
};

}   // namespace ReversibleLogic
