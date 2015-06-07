// ReversibleLogicGenerator - generator of reversible logic circuits, based on permutation group theory.
// Copyright (C) 2015  <Dmitry Zakablukov>
// E-mail: dmitriy.zakablukov@gmail.com
// Web: https://github.com/dmitry-zakablukov/ReversibleLogicGenerator
// 
// This file is part of ReversibleLogicGenerator.
// 
// ReversibleLogicGenerator is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ReversibleLogicGenerator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ReversibleLogicGenerator.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

namespace ReversibleLogic
{

class ReverseElement
{
public:
    ReverseElement();
    explicit ReverseElement( uint n, word targetMask, word controlMask = 0, word inversionMask = 0);

    virtual ~ReverseElement() = default;

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

    /// Returns true if elements are independent
    bool isSwappable(const ReverseElement& another) const;

    /// Returns true if 'this' element can be swapped with @another.
    /// If result is true, then @withOneControlLineInverting would be true,
    /// if swapping possible with inversion of one control line of 'this' element
    /// and would be false, if all inputs remain the same after swapping.
    /// If @withoutAnyChanges is true then return value is true only if elements are independent
    bool isSwappable(const ReverseElement& another, bool* withOneControlLineInverting,
        bool withoutAnyChanges = false) const;

    /// Returns true if 'this' element can be swapped with all elements from @elements
    /// while preserving all inputs the same
    bool isSwappable(const list<ReverseElement>& elements) const;

    void swap(ReverseElement* another);
    static void swap(ReverseElement* left, ReverseElement* right);

    word getValue(word input) const;

    deque<ReverseElement> getInversionOptimizedImplementation(bool heavyRight = true) const;

    /// All inversion inputs replaced by straights with additional NOT elements from left and right
    deque<ReverseElement> getSimpleImplementation() const;

    /// All inversion inputs replaced recursively by two elements: (0xx) -> (*xx)(1xx)
    deque<ReverseElement> getRecursiveImplementation() const;

    /// All k-CNOT elements replaced by 2-CNOT elements at most
    deque<ReverseElement> getToffoliOnlyImplementation(bool heavyRight = true) const;

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
