#pragma once

namespace ReversibleLogic
{
   
class SchemePrinter
{
public:
    template<typename Container>
    static string schemeToString(const Container& scheme,
        bool horizontal = true);

private:
    template<typename Container>
    static string horizontalPrint(const Container& scheme);

    template<typename Container>
    static string verticalPrint(const Container& scheme);
};

}   // namespace ReversibleLogic