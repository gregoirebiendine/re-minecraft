#ifndef INCLUDED_INSPECTION_HPP
#define INCLUDED_INSPECTION_HPP

#include "../CxxABI/CxxABI.hpp"
#include "../Headers.hpp"

namespace Flint
{
    /**
     * Base class to implement simplet self-reflexion in C++
     *
     * @ingroup flint
     *
     * @tparam T Child class itself
     *
     * @tip
     * When creating a child class, define as follow :
     * ```cpp
     * class ChildClass: public Inspection<ChildClass>
     * ```
     * This will extand the ChildClass with the self-reflexion class Inspection
     *
     * @warning
     * If the class you want to extand as a reflected class will be a parent
     * class (I.E. You will create childs of this "target" class), you might
     * want to mark inheritence as **virtual**. This is needed if your child
     * class _also_ need to be reflected. This way, there is no confilict
     * between the parent reflexion, and the child reflexion.
     */
    template<typename T>
    class Inspection
    {
        public:
            /**
             * Return the name of the reflected class
             *
             * @return The demangled name of the class, or just the name, if
             * demangleing failed
             */
            virtual std::string getClassName() const final
            {
                return CxxABI::demangle(typeid(T).name());
            }

            // virtual std::unordered_set<std::pair<std::string, std::string>>
            // getAttributes();

            /**
             * Return a string representation of the class
             *
             * @tip
             * This function is not intended to be used.
             * This is because this functions display the reflection.
             * You might prefer to override this function in yout child
             * class to display informations that are more relevant to
             * your specific class.
             *
             * @example
             * You have a Macro available to make the str function really
             * easilly :
             * ```cpp
             * display_attr(attribute)
             * ```
             * Which will automaticly translate to
             * ```cpp
             * Flint::Colors::F_BLUE << "attribute" << Flint::Colors::RESET <<
             * "=" << Flint::Colors::RED << this->attribute <<
             * Flint::Colors::RESET
             * ```
             * There is also a `make_str` macro, which you can use as follow :
             * ```cpp
             * return make_str("a=12" << ", " << "b=5");
             * ```
             * that becomes
             * ```cpp
             * return ({std::stringstream ss; ss << "<" << this->getClassName()
             * << " at " << std::hex << this << ": " << "a=12" << ", " << "b=5"
             * << ">"; ss.str()})
             * ```
             * It means you can define the str function of your class as like
             * that :
             * ```cpp
             * // Provided you have 3 attributes called a, b and c
             * return make_str(display_attr(a) << ", " << display_attr(b) << ",
             * " << display_attr(c));
             * ```
             *
             * @return The string representation of the class
             */
            virtual std::string str() const
            {
                std::stringstream ss;

                ss << "<" << Flint::Colors::F_PURPLE << this->getClassName()
                   << Flint::Colors::RESET << " at " << Flint::Colors::YELLOW
                   << std::hex << this << Flint::Colors::RESET << ">";

                return ss.str();
            }

            /** Overload the operator << to use the str method of the object
             *
             * @tparam T Reflected class
             *
             * @param os Reference of ostream (I.E. std::cout, std::cerr...)
             *
             * @param obj Object reference to be displayed
             *
             * @return Param os
             */
            friend std::ostream& operator<<(std::ostream&        os,
                                            const Inspection<T>& obj)
            {
                os << obj.str();
                return os;
            }
    };
}


#endif
