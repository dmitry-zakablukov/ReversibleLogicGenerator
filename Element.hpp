#pragma once

namespace ReversibleLogic
{

struct FinalPair
{
    FinalPair();
    FinalPair(word theTargetMask, word theControlMask);

    word targetMask;
    word controlMask;
};

class ReverseElement
{
public:
    ReverseElement();
    explicit ReverseElement( uint n, word targetMask,
                             word controlMask = 0, word inversionMask = 0);

    void setInputCount(uint count);
    uint getInputCount() const;

    void setIndependencyFlag(bool independency);
    bool isIndependent() const;

    void setTargetMask(word theTargetMask);
    word getTargetMask() const;

    void setControlMask(word theControlMask);
    word getControlMask() const;

    void setInversionMask(word theInversionMask);
    word getInversionMask() const;

    bool operator ==(const ReverseElement& another) const;

    bool isSymmetric() const;

    bool isValid() const;

    bool isSwitchable(const ReverseElement& another) const;
    bool isSwitchable(const list<ReverseElement>& elements) const;

    word getValue(word input) const;

    deque<ReverseElement> getInversionOptimizedImplementation(bool heavyRight = true) const;
    deque<ReverseElement> getImplementation(bool heavyRight = true) const;

    ReverseElement getLeftmostElement(bool heavyRight = true) const;
    ReverseElement getRightmostElement(bool heavyRight = true) const;

    ReverseElement getFinalImplementation() const;

private:
    word getFreeInputMask() const;

    uint n;
    bool independencyFlag;

    word targetMask;
    word controlMask;
    word inversionMask;
};

typedef deque<ReverseElement> Scheme;

}   // namespace ReversibleLogic
