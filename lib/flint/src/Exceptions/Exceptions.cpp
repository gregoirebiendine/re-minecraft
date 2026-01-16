#include "Exceptions.hpp"

namespace Flint::Exceptions
{
    Exception::Exception(
      const std::string&                                          what,
      std::pair<std::pair<std::string, std::size_t>, std::string> infos):
        _infos(infos)
    {
        std::vector<std::pair<std::string, std::size_t>> stackTrace =
          CxxABI::getBacktrace();
        std::stringstream ss;

        ss << "[" << Flint::Colors::F_PURPLE
           << this->getClassNameFromStackTrace(stackTrace)
           << Flint::Colors::RESET;

        if (!infos.first.first.empty() && !infos.second.empty()) {
            ss << " in " << Flint::Colors::F_YELLOW << infos.second
               << Flint::Colors::RESET << " at " << Flint::Colors::F_BLUE
               << infos.first.first.substr(
                    infos.first.first.find_last_of("/\\") + 1)
               << Flint::Colors::RESET << ":" << Flint::Colors::F_RED
               << infos.first.second << Flint::Colors::RESET;
        }
        ss << "] " << what;

        for (std::size_t i = 2; i < stackTrace.size(); i++) {
            std::string funcName = stackTrace[i].first;
            std::size_t openP    = funcName.find('(');
            if (openP == std::string::npos) funcName += "()";
            funcName = funcName.substr(0, funcName.find("(") + 1) + "[...])";
            ss << std::endl
               << " -> " << Flint::Colors::CYAN << funcName
               << Flint::Colors::RESET << ":" << Flint::Colors::YELLOW << "0x"
               << std::hex << stackTrace[i].second << Flint::Colors::RESET;
        }

        this->_what = ss.str();
    }

    std::pair<std::pair<std::string, std::size_t>, std::string>
      Exception::getInfos() const
    {
        return this->_infos;
    }

    std::string Exception::getClassNameFromStackTrace(
      const std::vector<std::pair<std::string, std::size_t>>& stackTrace) const
    {
        if (stackTrace.size() <= 2) return "[Class name undefined]";

        std::string classNameSymbol = stackTrace[2].first.substr();
        std::size_t openP           = classNameSymbol.find('(');
        if (openP == std::string::npos) return "[Class name undefined]";

        std::string functionName = classNameSymbol.substr(0, openP);
        std::size_t lastColon    = functionName.rfind("::");
        if (lastColon == std::string::npos) return "[Class name undefined]";
        // Remove duplicate
        return functionName.substr(0, lastColon);
    }

    std::string Exception::show(std::pair<std::pair<std::string, std::size_t>,
                                          std::string> fileInfos) const
    {
        std::stringstream ss;
        ss << "Exception caught on " << Flint::Colors::F_GREEN
           << fileInfos.second << Flint::Colors::RESET << " at "
           << Flint::Colors::F_BLUE << fileInfos.first.first
           << Flint::Colors::RESET << ":" << Flint::Colors::F_RED
           << fileInfos.first.second << Flint::Colors::RESET << std::endl
           << std::endl;

        ss << this->_what;
        return ss.str();
    }

    const char* Exception::what() const noexcept
    {
        return this->_what.c_str();
    }

    std::ostream& operator<<(std::ostream& os, const Exceptions::Exception& obj)
    {
        os << obj.what();
        return os;
    }
}
