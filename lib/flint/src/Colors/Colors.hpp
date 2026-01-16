#ifndef INCLUDED_COLORS_HPP
#define INCLUDED_COLORS_HPP

#include "../Headers.hpp"

namespace Flint
{
    /**
     * This class defined a good set of ANSI colors.
     *
     * @ingroup flint
     *
     * @tip
     * It is recomended to use those through streams :
     * std::cout << Flint::Colors::GREEN << "Hacker time" <<
     * Flint::Colors::RESET << std::endl;
     *
     * @info
     * Please ALWAYS remember to close your colors with Flint::Colors::RESET.
     * Failure to do si might result in broken displays, with weird colors
     * everywhere
     */
    class Colors
    {
        public:
            static const std::string RESET;
            static const std::string BOLD;
            static const std::string DISABLED;
            static const std::string ITALIC;
            static const std::string UNDERLINE;
            static const std::string BLINK;
            static const std::string HIGHLIGHTED;
            static const std::string STRIPED;
            static const std::string BLACK;
            static const std::string RED;
            static const std::string GREEN;
            static const std::string YELLOW;
            static const std::string BLUE;
            static const std::string PURPLE;
            static const std::string CYAN;
            static const std::string GREY;
            static const std::string F_BLACK;
            static const std::string F_RED;
            static const std::string F_GREEN;
            static const std::string F_YELLOW;
            static const std::string F_BLUE;
            static const std::string F_PURPLE;
            static const std::string F_CYAN;
            static const std::string F_GREY;
            static const std::string B_BLACK;
            static const std::string B_RED;
            static const std::string B_GREEN;
            static const std::string B_YELLOW;
            static const std::string B_BLUE;
            static const std::string B_PURPLE;
            static const std::string B_CYAN;
            static const std::string B_GREY;
    };
}

#endif
