#ifndef INCLUDED_EXCEPTIONS_HPP
#define INCLUDED_EXCEPTIONS_HPP

#include "../Colors/Colors.hpp"
#include "../CxxABI/CxxABI.hpp"
#include "../Headers.hpp"
#include "../hpp/Preprocessor.hpp"

namespace Flint::Exceptions
{
    /**
     * Wrapper arround default
     * [std::exception](https://en.cppreference.com/w/cpp/error/exception)
     *
     * @ingroup flint
     *
     * Store the the exception description as
     * [std::string](https://en.cppreference.com/w/cpp/string/basic_string), and
     * store some usefull localisation informations: Filename, Line, and
     * Function Name. This is automated using the throw_exception preprocessor
     * directive.
     *
     * @tip
     * Take a look at the NotImplementedError to see how easy it is to implmeent
     * your own Flint::Exception.
     */
    class Exception: public std::exception
    {
        protected:
            std::string                                                 _what;
            std::pair<std::pair<std::string, std::size_t>, std::string> _infos;

        public:
            /**
             * Exception constructor. This need to be called, even on child
             * classes.
             *
             * @param what Define the exception's description
             * @param infos Might contain positional infos
             *
             * @warning
             * You should **NOT** use any excpetion constructor manually.
             * While they might work, the prefered way is to use the
             * throw_exception preprocessor directive. Using it will only
             * require you to register what type of excpetion you want to throw,
             * and what message you want to add, and will take care of putting
             * file name, line, and function name
             */
            Exception(const std::string&     what  = "An exception occured !",
                      std::pair<std::pair<std::string, std::size_t>,
                                std::string> infos = {
                          {"", 0},
                          ""
            });

            /**
             * Retreive given infos (filename, line, and function name)
             *
             * @return The actual infos
             */
            std::pair<std::pair<std::string, std::size_t>, std::string>
              getInfos() const;

            /**
             * Try to retreive the Exception class name from the StackTrace.
             *
             * @param stackTrace A stack trace, formated like
             * CxxABI::getBacktrace()
             *
             * @warning
             * This function **DOES NOT WORK 100% OF THE TIME**.
             * It is really dependant on the environement, and can return
             * undefined without aparent reason. It should **NOT** be used for
             * anything else than debug or display.
             *
             * @return The actual class name or "[Class name undefined]"
             */
            std::string getClassNameFromStackTrace(
              const std::vector<std::pair<std::string, std::size_t>>&
                stackTrace) const;

            /**
             * Display the error, given specific positional infos (like the
             * constructor).
             *
             * @param infos Positional informations, like the constructor, used
             * to display the infos.
             *
             * @note
             * This function, while being usable without, should be used through
             * the catch_exception preprocessor directive. This preprocessor
             * directive let you display Python like excpetion trace.
             *
             * @return The formated exception, ready to be displayed.
             */
            std::string show(std::pair<std::pair<std::string, std::size_t>,
                                       std::string> infos) const;

            /**
             * Required override, deprecated by show and catch_exception.
             *
             * @info
             * This is the required std::exception override.
             * It'll return the _what as a const char*.
             * While this will work, prefer using the show function or the
             * catch_excpetion preprocessor directive :
             *
             * @example
             * ```cpp
             * try {
             *     throw_exception(Flint::Exceptions::Exception, "Sample testing
             * exception"); } catch (const Flint::Exceptions::Exception& e) {
             *     std::cerr << catch_exception(e) << std::endl;
             * }
             * ```
             *
             * @return The _what as a C string
             */
            const char* what() const noexcept override;

            /**
             * operator<< override
             *
             * @abstract
             * This will simply append the _what to the ostream
             */
            friend std::ostream& operator<<(std::ostream&                os,
                                            const Exceptions::Exception& obj);
    };

    /**
     * Sample Flint::Exceptions::Exception to register NotImplemented features
     *
     * @ingroup flint
     *
     * @tip
     * You should use that Exception as a C++ Implementation of the Rust todo!()
     * macro.
     *
     * @example
     * ```cpp
     * throw_exception(Flint::Exceptions::NotImplementedError, "This function
     * haven't been implemented as of today !");
     * ```
     */
    class NotImplementedError: public Flint::Exceptions::Exception
    {
        public:
            NotImplementedError(const std::string& what =
                                  "Asked action haven't been implemented yet.",
                                std::pair<std::pair<std::string, std::size_t>,
                                          std::string> infos = {{"", 0}, ""}):
                Exception(what, infos)
            {
            }
    };
}

#endif
