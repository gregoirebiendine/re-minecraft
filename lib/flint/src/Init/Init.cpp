#include "Init.hpp"

namespace Flint
{
    static void unhandeled_exception()
    {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            const Flint::Exceptions::Exception* exception =
              dynamic_cast<const Flint::Exceptions::Exception*>(&e);
            if (exception != nullptr) {
                std::cerr << Flint::Colors::RED << "/!\\"
                          << Flint::Colors::RESET
                          << " Uncaught Flint exception " << Flint::Colors::RED
                          << "/!\\" << Flint::Colors::RESET << std::endl
                          << std::endl
                          << *exception << std::endl;
            } else {
                std::cerr << Flint::Colors::RED << "/!\\"
                          << Flint::Colors::RESET << " Uncaught exception "
                          << Flint::Colors::RED << "/!\\"
                          << Flint::Colors::RESET << std::endl
                          << std::endl
                          << e.what() << std::endl;
            }
        }
    }

    void Init()
    {
        std::set_terminate(unhandeled_exception);
    }
}
