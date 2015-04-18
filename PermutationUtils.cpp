#include "std.hpp"

namespace ReversibleLogic
{

Permutation PermutationUtils::createPermutation(const TruthTable& table,
    bool permutationShouldBeEven /*= true*/)
{
    vector<Piece> pieces = findPieces(table);
    vector<Piece> cycles = mergePieces(pieces);

    uint cycleCount = cycles.size();

    Permutation permutation;
    for(uint cycleId = 0; cycleId < cycleCount; ++cycleId)
    {
        Piece& piece = cycles[cycleId];
        permutation.append( shared_ptr<Cycle>(new Cycle(move(piece))) );
    }

    if (permutationShouldBeEven && !permutation.isEven())
    {
        const ProgramOptions& options = ProgramOptions::get();
        if (options.isTuningEnabled && options.options.getBool("complete-permutation-to-even", false))
        {
            word tableSize = table.size();
            permutation.completeToEven(tableSize);

            assert(permutation.isEven(), string("Can't complete permutation to even"));
        }
    }

    return permutation;
}

vector<PermutationUtils::Piece> PermutationUtils::findPieces(const TruthTable& inputTable)
{
    TruthTable table = inputTable;
    vector<Piece> pieces;

    uint transformCount = table.size();
    for(word x = 0; x < transformCount; ++x)
    {
        word& y = table[x];
        if(y == wordUndefined)
            continue;

        if(x == y)
        {
            y = wordUndefined;
            continue;
        }

        // count piece length
        word length = 1;
        word z = y;

        while(z != x)
        {
            assertd(z < transformCount, string("Too big index"));
            word& temp = table[z];

            ++length;
            assertd(length < transformCount, string("Too big piece"));

            if(temp != z && temp != wordUndefined)
                z = temp;
            else
                break;
        }

        // fill piece
        bufferize(pieces);
        pieces.push_back(Piece());

        Piece& piece = pieces.back();
        piece.resize(length);

        piece[0] = x;

        word index = 1;
        z = y;

        while(z != x)
        {
            piece[index] = z;

            word& temp = table[z];
            if(temp != z && temp != wordUndefined)
            {
                z = temp;
                temp = wordUndefined;

                ++index;
            }
            else
            {
                temp = wordUndefined;
                break;
            }
        }

        y = wordUndefined;
    }

    return pieces;
}

vector<PermutationUtils::Piece> PermutationUtils::mergePieces(const vector<Piece>& pieces)
{
    bool repeat = true;
    vector<Piece> inputCycles = pieces;
    vector<Piece> outputCycles;

    while(repeat)
    {
        repeat = false;
        outputCycles = vector<Piece>();

        uint inputCount = inputCycles.size();
        for(word inputId = 0; inputId < inputCount; ++inputId)
        {
            Piece& piece = inputCycles[inputId];

            word& firstElement = piece.front();
            word& lastElement  = piece.back();

            bool merged = false;

            uint outputCount = outputCycles.size();
            for(word outputId = 0; outputId < outputCount; ++outputId)
            {
                Piece& cycle = outputCycles[outputId];
                word& cycleStart = cycle.front();
                word& cycleEnd   = cycle.back();

                if(cycleStart == lastElement)
                {
                    uint cycleLength = cycle.size();
                    uint pieceLength = piece.size();

                    uint length = cycleLength + pieceLength - 1;
                    cycle.resize(length);

                    word* cyclePtr = &cycle[0];
                    memcpy(cyclePtr + pieceLength - 1, cyclePtr, cycleLength * sizeof(word));

                    word* piecePtr = &piece[0];
                    memcpy(cyclePtr, piecePtr, (pieceLength - 1) * sizeof(word));

                    merged = true;
                }
                else if(cycleEnd == firstElement)
                {
                    uint cycleLength = cycle.size();
                    uint pieceLength = piece.size();

                    uint length = cycleLength + pieceLength - 1;
                    cycle.resize(length);

                    word* cyclePtr = &cycle[0];
                    word* piecePtr = &piece[0];
                    memcpy(cyclePtr + cycleLength, piecePtr + 1, (pieceLength - 1) * sizeof(word));

                    merged = true;
                }

                if(merged)
                {
                    repeat = true;
                    break;
                }
            }

            if(!merged)
            {
                bufferize(outputCycles);
                outputCycles.push_back(piece);
            }
        }

        swap(inputCycles, outputCycles);
    }

    swap(inputCycles, outputCycles);
    return outputCycles;
}

}   // namespace ReversibleLogic
