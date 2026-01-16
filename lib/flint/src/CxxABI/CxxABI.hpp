#ifndef INCLUDED_CXXABI_HPP
#define INCLUDED_CXXABI_HPP

#include "../Headers.hpp"

namespace Flint
{
    /**
     * Wrapper around usefull C++/C low level functions
     *
     * @ingroup flint
     *
     * Got demangle, backtrace...
     *
     * @tip
     * This class is only made of static functions. It does not contain
     * constructor, nor destructor This means that you don't need to instanciate
     * it, and only use it as if it was a namespace.
     */
    class CxxABI
    {
        public:
            CxxABI() = delete;

            /**
             * Demangle a C++ name.
             *
             * This is usefull for : Class names, backtrace...
             *
             * @return string containing : demangled name, or original name, if
             * demangle did not work
             */
            static std::string demangle(const char* name);

            /**
             * Get functions infos from symbol extraction
             *
             * @return A pair with the function names demangled, and the offset
             * from the function
             *
             * @warning
             * The function name is not EXACTLY the real name of the function.
             * Also, the offset DOES NOT correspond to the line number, but the
             * offset of asm instructions
             */
            static std::pair<std::string, std::size_t>
              getFuncInfos(const char* symbolInfos);

            /**
             * Retreive the execution trace
             *
             * @return Vector of pair<pair<string, size_t>, string>. I'll
             * explain
             *
             * The idea is to retrieive the file, line, and function. So that's
             * what we do : pair or : (file, line), function
             *
             * @warning
             * To correctly grab function names, you will need to add
             * compilation flag "-rdynamic" See this lib's CMakeLists.txt for
             * implementation. This will export ALL symbols. If NOT done, you
             * will get offsets from binary. It's ok, but quite disgusting on
             * output.
             */
            static std::vector<std::pair<std::string, std::size_t>>
              getBacktrace();
    };
}

#endif
