#include "Logging.hpp"

#include <optional>

namespace Flint
{
    Logging& Logging::get()
    {
        static Logging instance;
        return instance;
    }

    void Logging::set_log_on_stderr(bool log_on_stderr)
    {
        Logging::get().log_on_stderr = log_on_stderr;
    }

    void Logging::set_log_level(LogLevel level)
    {
        Logging::get().display_level = level;
    }

    void Logging::set_error_level(LogLevel level)
    {
        Logging::get().error_level = level;
    }

    void Logging::display_positional_infos(
      std::pair<std::pair<std::string, std::size_t>, std::string>
        positional_infos)
    {
        std::cout << "(at " << Flint::Colors::CYAN
                  << positional_infos.first.first << Flint::Colors::RESET << ":"
                  << Flint::Colors::PURPLE << positional_infos.first.second
                  << Flint::Colors::RESET << " in " << Flint::Colors::BLUE
                  << positional_infos.second << Flint::Colors::RESET << ") ";
    }

    void Logging::debug(
      const std::string& message,
      std::optional<std::pair<std::pair<std::string, std::size_t>, std::string>>
        positional_infos)
    {
        std::cout << "[ " << Flint::Colors::GREY << "DEBG"
                  << Flint::Colors::RESET << " ] ";

        if (positional_infos)
            Logging::display_positional_infos(*positional_infos);

        std::cout << message << std::endl;

        auto backtrace = Flint::CxxABI::getBacktrace();

        for (auto entry: backtrace)
            std::cout << "    in " << Flint::Colors::PURPLE << entry.first
                      << Flint::Colors::RESET << ":" << Flint::Colors::GREEN
                      << entry.second << Flint::Colors::RESET << std::endl;
        std::cout << "== End Debug Backtrace ==" << std::endl;

        if (Logging::get().error_level <= Flint::LogLevel::DEBUG)
            throw_exception(Flint::TooHighLevelLoggingError, message);
    }

    void Logging::info(
      const std::string& message,
      std::optional<std::pair<std::pair<std::string, std::size_t>, std::string>>
        positional_infos)
    {
        std::cout << "[ " << Flint::Colors::GREEN << "INFO"
                  << Flint::Colors::RESET << " ] ";

        if (positional_infos)
            Logging::display_positional_infos(*positional_infos);

        std::cout << message << std::endl;

        if (Logging::get().error_level <= Flint::LogLevel::INFO)
            throw_exception(Flint::TooHighLevelLoggingError, message);
    }

    void Logging::warning(
      const std::string& message,
      std::optional<std::pair<std::pair<std::string, std::size_t>, std::string>>
        positional_infos)
    {
        std::cout << "[ " << Flint::Colors::YELLOW << "WARN"
                  << Flint::Colors::RESET << " ] ";

        if (positional_infos)
            Logging::display_positional_infos(*positional_infos);

        std::cout << message << std::endl;

        if (Logging::get().error_level <= Flint::LogLevel::WARNING)
            throw_exception(Flint::TooHighLevelLoggingError, message);
    }

    void Logging::error(
      const std::string& message,
      std::optional<std::pair<std::pair<std::string, std::size_t>, std::string>>
        positional_infos)
    {
        std::cout << "[ " << Flint::Colors::RED << "FAIL"
                  << Flint::Colors::RESET << " ] ";

        if (positional_infos)
            Logging::display_positional_infos(*positional_infos);

        std::cout << message << std::endl;

        if (Logging::get().error_level <= Flint::LogLevel::ERROR)
            throw_exception(Flint::TooHighLevelLoggingError, message);
    }

    void Logging::critical(
      const std::string& message,
      std::optional<std::pair<std::pair<std::string, std::size_t>, std::string>>
        positional_infos)
    {
        std::cout << "[ " << Flint::Colors::B_RED << "CRIT"
                  << Flint::Colors::RESET << " ] ";

        if (positional_infos)
            Logging::display_positional_infos(*positional_infos);

        std::cout << message << std::endl;

        if (Logging::get().error_level <= Flint::LogLevel::CRITICAL)
            throw_exception(Flint::TooHighLevelLoggingError, message);
    }
}
