#ifndef INCLUDED_LOGGING_HPP
#define INCLUDED_LOGGING_HPP

#include "../Colors/Colors.hpp"
#include "../CxxABI/CxxABI.hpp"
#include "../Exceptions/Exceptions.hpp"
#include "../Headers.hpp"

#include <optional>

namespace Flint
{
    enum LogLevel {
        DEBUG    = 0,
        INFO     = 1,
        WARNING  = 2,
        ERROR    = 3,
        CRITICAL = 4,
        NONE     = 5,
    };

    class TooHighLevelLoggingError: public Flint::Exceptions::Exception
    {
        public:
            TooHighLevelLoggingError(
              const std::string& what =
                "The message logged is of criticly high level.",
              std::pair<std::pair<std::string, std::size_t>, std::string>
                infos = {{"", 0}, ""}):
                Exception(what, infos)
            {
            }
    };

    class Logging
    {
        private:
            Logging() {}

            bool     log_on_stderr = false;
            LogLevel display_level = LogLevel::INFO;
            LogLevel error_level   = LogLevel::CRITICAL;

            static Logging& get();

        public:
            Logging(const Logging&)        = delete;
            void operator=(const Logging&) = delete;

            static void set_log_on_stderr(bool log_on_stderr);
            static void set_log_level(LogLevel level);
            static void set_error_level(LogLevel level);

            static void display_positional_infos(
              std::pair<std::pair<std::string, std::size_t>, std::string>
                positional_infos);

            static void
              debug(const std::string&                    message,
                    std::optional<std::pair<std::pair<std::string, std::size_t>,
                                            std::string>> positional_infos =
                      std::nullopt);

            static void
              info(const std::string&                    message,
                   std::optional<std::pair<std::pair<std::string, std::size_t>,
                                           std::string>> positional_infos =
                     std::nullopt);

            static void warning(
              const std::string& message,
              std::optional<
                std::pair<std::pair<std::string, std::size_t>, std::string>>
                positional_infos = std::nullopt);

            static void
              error(const std::string&                    message,
                    std::optional<std::pair<std::pair<std::string, std::size_t>,
                                            std::string>> positional_infos =
                      std::nullopt);

            static void critical(
              const std::string& message,
              std::optional<
                std::pair<std::pair<std::string, std::size_t>, std::string>>
                positional_infos = std::nullopt);
    };
}

#endif
