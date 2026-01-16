#ifndef INCLUDED_INIT_HPP
#define INCLUDED_INIT_HPP

#include "../Exceptions/Exceptions.hpp"

namespace Flint
{
    [[gnu::constructor]] void Init();
}

#endif
