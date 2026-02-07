#include "CxxABI.hpp"

namespace Flint
{
    std::string CxxABI::demangle(const char* name)
    {
#ifdef _WIN32
        std::cerr << "Called demangle on windows. This is not supported (lol yall have a weird a$$ abi)" << std::cerr;
        return "";
#else
        int   status = -4;
        char* _out   = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        if (status != 0 && _out) free(_out);
        if (status != 0) return std::string(name);
        std::string result(_out);
        free(_out);
        return result;
#endif
    }

    std::pair<std::string, std::size_t>
      CxxABI::getFuncInfos(const char* symbolInfos)
    {
        std::pair<std::string, std::size_t> output;
#ifdef _WIN32
        std::cerr << "Called getFuncInfos on windows. This is not supported (would have worked on rust tbh)" << std::cerr;
        return output;
#else
        std::string                         entry(symbolInfos);

        std::size_t openP  = entry.find('(');
        std::size_t closeP = entry.find(')');

        if (openP == std::string::npos || closeP == std::string::npos)
            return output;

        std::string functionAndOffset = entry.substr(openP + 1, closeP);
        std::size_t plusP             = functionAndOffset.find('+');

        if (functionAndOffset.empty() || plusP == std::string::npos)
            return output;

        output.first =
          CxxABI::demangle(functionAndOffset.substr(0, plusP).c_str());
        std::stringstream ss;
        ss << std::hex << functionAndOffset.substr(plusP + 1);
        ss >> output.second;

        return output;
#endif
    }

    // std::vector<std::pair<std::pair<std::string, std::size_t>, std::string>>
    // CxxABI::getBacktrace()
    std::vector<std::pair<std::string, std::size_t>> CxxABI::getBacktrace()
    {
        std::vector<std::pair<std::string, std::size_t>> out;
#ifdef _WIN32
        std::cerr << "Called getBacktrace on windows. This is not supported (because of shitty abi)" << std::cerr;
        return out;
#else
        int                                              TRACE_CHUNCK = 128;

        int size = 0, out_size = 0;
        do {
            size += TRACE_CHUNCK;
            void* trace[size + 1];
            out_size = (int)backtrace(trace, size);
        } while (out_size == size);
        void* trace[out_size + 1];

        backtrace(trace, out_size);
        char** symbols = backtrace_symbols(trace, out_size);
        if (symbols) {
            for (int i = 0; i < out_size; ++i) {
                std::pair<std::string, std::size_t> symbol =
                  CxxABI::getFuncInfos(symbols[i]);
                if (symbol.first.empty()) break;
                out.push_back(symbol);
            }
            free(symbols);
        }

        return out;
#endif
    }
}
