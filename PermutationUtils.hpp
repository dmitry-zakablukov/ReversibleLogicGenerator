#pragma once

namespace ReversibleLogic
{

class PermutationUtils
{
public:
    typedef vector<word> Piece;

    static Permutation createPermutation(const PermutationTable& inputTable);

private:
    // Applies various optimization techniques to the table
    static PermutationTable expandTable(const PermutationTable& table);

    static vector<Piece> findPieces(const PermutationTable& table);
    static vector<Piece> mergePieces(const vector<Piece>& pieces);
};

}   // namespace ReversibleLogic
