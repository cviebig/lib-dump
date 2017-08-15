#ifndef DUMP_CONSOLE_HPP
#define DUMP_CONSOLE_HPP

#include "dump/x3_support.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>

#include <array>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace dump {

    /**
     * Struct that keeps the state of the console_dumper.
     */
    struct dumper_state {
        std::ostream& out;
        std::string prefix;

        dumper_state(std::ostream& out,
                     std::string prefix)
            : out(out), prefix(prefix) {}
    };

    /**
     * Base class for data structure dumpers.
     *
     * We use the Curiously recurring template pattern here to be able
     * to call method's of the derived class.
     */
    template <typename Derived>
    struct console_dumper {
        typedef console_dumper<Derived> Self;

        const std::string black = "\x1B[31m";
        const std::string red = "\x1B[31m";
        const std::string green = "\x1B[32m";
        const std::string yellow = "\x1B[33m";
        const std::string blue = "\x1B[34m";
        const std::string magenta = "\x1B[35m";
        const std::string cyan = "\x1B[36m";
        const std::string white = "\x1B[37m";

        const std::string bold = "\x1B[1m";

        const std::string reset = "\x1B[0m";

        dumper_state state;
        std::string label;
        bool last_child;
        bool inlined;

        console_dumper(std::ostream& out)
            : state(out, ""), label(""), last_child(true),
              inlined(false) {}

        console_dumper(dumper_state state, std::string label,
                       bool last_child, bool inlined)
            : state(state), label(label), last_child(last_child),
              inlined(inlined) {
            print_prefix();
        }

        Derived &getDerived() {
            return static_cast<Derived&>(*this);
        }

        const Derived &getDerived() const {
            return static_cast<const Derived&>(*this);
        }

        void print_prefix() {
            if(!inlined) {
                state.out << blue;
                state.out << std::endl;
                state.out << state.prefix;
                state.out << (last_child ? '`' : '|');
                state.out << "- ";
                state.out << reset;
                state.prefix = state.prefix + (last_child ? " " : "|") + " ";
            } else {
                state.out << ' ';
            }
            if(label.length() > 0) {
                state.out << label << ' ';
            }
        }

        template <typename Node>
        void cNode(Node const& node, std::string const& text) {
            state.out << bold << magenta << text << reset;
        }

        template <typename Dumper, typename Parent, typename Child>
        void cAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {
            Dumper{state, label, last_child, inlined}(child);
        }

        template <typename Parent, typename Child>
        void cAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {
            Derived{state, label, last_child, inlined}(child);
        }

        template <typename T, std::size_t N>
        void cList(std::array<T, N> const& ops, std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename T>
        void cList(std::vector<T> const& ops, std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename T, typename Compare, typename Allocator>
        void cList(std::set<T, Compare, Allocator> const& ops,
                   std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename Key, typename T, typename Compare, typename Allocator>
        void cList(std::map<Key, T, Compare, Allocator> const& ops,
                   std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename... _Args>
        void cList(boost::multi_index_container<_Args...> const& ops,
                   std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename T, std::size_t A>
        void cList(boost::container::small_vector<T, A> const& ops,
                   std::string const& text) {
            cNode(ops, text);
            (*this)(ops);
        }

        template <typename T>
        void cAddr(T const& obj) {
            state.out << ' ' << cyan << std::addressof(obj) << reset;
        }

        template <typename T>
        void cType(T const& obj) {
            // alternatively use typeid(T).name() which is not
            // required to be human readable
            state.out << ' '
                      << yellow
                      << boost::typeindex::type_id<T>().pretty_name()
                      << reset;
        }

        template <typename T>
        void cTypeAddr(T const& obj) {
            cType(obj);
            cAddr(obj);
        }

        void cEnum(std::string const& text) {
            state.out << yellow << text << reset;
        }

        void cValue(std::string const& text) {
            state.out << green << "'" << text << "'" << reset;
        }

        void cBlock(std::string text) {
            const std::string replacement
                = "\n" + blue + state.prefix + "  " + reset;
            state.out << std::endl
                      << blue << state.prefix << "`- " << reset
                      << boost::replace_all_copy(text,
                                                 "\n",
                                                 replacement);
        }

        void cErr(std::string const& text) {
            state.out << bold << red << text << reset;
        }

        template <typename T>
        void cErrTypeAddr(std::string const& text, T const& obj) {
            cErr(text);
            cTypeAddr(obj);
        }

        void cUndefined() {
            state.out << magenta << "undefined" << reset;
        }

        void operator()(bool const& value)
        {
            cValue((value ? "true" : "false"));
        }

        void operator()(char const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(unsigned char const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(short const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(unsigned short const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(int const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(unsigned int const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(long const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(unsigned long const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(float const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(double const& value)
        {
            cValue(std::to_string(value));
        }

        void operator()(std::string const& value)
        {
            // We expect inlined string attributes to be one liners
            if(inlined) {
                cValue(value);
            } else {
                const std::string replacement
                    = "\n"
                    // Reset formatting
                    + reset
                    // Prefix formatting and prefix content
                    + blue + state.prefix
                    // Label indentation +2 for spaces around
                    + std::string(label.size()+2, ' ')
                    // Reset formatting again
                    + reset
                    // restore cValue formatting
                    + green;
                state.out << green
                          << boost::replace_all_copy(value,
                                                     "\n",
                                                     replacement)
                          << reset;
            }
        }

        template <typename T>
        void operator()(std::shared_ptr<T> const& op) {
            if(op != nullptr) {
                getDerived()(*op);
            } else {
                cUndefined();
            }
        }

        template <typename... _Args>
        void operator()(boost::variant<_Args...> const& op) {
            return boost::apply_visitor(getDerived(), op);
        }

        template <typename T>
        void operator()(boost::optional<T> const& op) {
            if(op) {
                getDerived()(op.get());
            } else {
                cUndefined();
            }
        }

        // Boost Spirit X3 variant type
        template <typename T,
              typename std::enable_if<is_variant<T>::value, bool>::type = false>
        void operator()(T const& ast) {
            boost::apply_visitor(getDerived(), ast);
        }

        template <typename T>
        void operator()(boost::spirit::x3::forward_ast<T> const& ast) {
            getDerived()(ast.get());
        }

        template <typename T, std::size_t N>
        void operator()(std::array<T, N> const& ops) {
            auto s = ops.size();
            cAttr<Derived>(ops, "size", s, false, true);
            for(auto const& op : ops) {
                cAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename T, typename Allocator>
        void operator()(std::vector<T, Allocator> const& ops) {
            auto s = ops.size();
            cAttr<Derived>(ops, "size", s, false, true);
            for(auto const& op : ops) {
                cAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename Key, typename Compare, typename Allocator>
        void operator()(std::set<Key, Compare, Allocator> const& ops) {
            auto s = ops.size();
            cAttr<Derived>(ops, "size", s, false, true);
            for(auto const& op : ops) {
                cAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename Key, typename T, typename Compare, typename Allocator>
        void operator()(std::map<Key, T, Compare, Allocator> const& ops) {
          auto s = ops.size();
          cAttr<Derived>(ops, "size", s, false, true);
          for(auto const& op : ops) {
              cAttr<Derived>(ops, "item", op, --s==0);
          }
        }

        template <typename Key, typename T, typename Compare, typename Allocator>
        void operator()(typename std::map<Key, T, Compare, Allocator>::value_type const& op) {
            cNode("Item", op);
            cAttr<Derived>(op, "key", op.first, false);
            cAttr<Derived>(op, "value", op.second, true);
        }

        template <typename... _Args>
        void operator()(boost::multi_index_container<_Args...>
                        const& ops) {
          auto s = ops.size();
          cAttr<Derived>(ops, "size", s, false, true);
          for(auto const& op : ops) {
              cAttr<Derived>(ops, "item", op, --s==0);
          }
        }

        template <typename T, std::size_t N, typename Allocator>
        void operator()(boost::container::small_vector<T, N, Allocator> const& ops) {
            auto s = ops.size();
            cAttr<Derived>(ops, "size", s, false, true);
            for(auto const& op : ops) {
                cAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename T,
              typename std::enable_if<!is_variant<T>::value, bool>::type = false>
        void operator()(T const& ast) {
            cErrTypeAddr("not implemented", ast);
        }
    };

}

#endif //DUMP_CONSOLE_HPP
