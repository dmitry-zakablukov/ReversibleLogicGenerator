#pragma once

namespace ReversibleLogic
{
   
class SchemePrinter
{
public:
    static string schemeToString(const deque<ReverseElement>& scheme,
        bool horizontal = true);

private:
    static string horizontalPrint(const deque<ReverseElement>& scheme);
    static string verticalPrint(const deque<ReverseElement>& scheme);
};

}   // namespace ReversibleLogic