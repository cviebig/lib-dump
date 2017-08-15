#ifndef DUMP_PLAINTEXT_HPP
#define DUMP_PLAINTEXT_HPP

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
    struct plaintext_dumper_state {
        std::ostream& out;
        std::string prefix;

        plaintext_dumper_state(std::ostream& out,
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
    struct plaintext_dumper {
        typedef plaintext_dumper<Derived> Self;

        plaintext_dumper_state state;
        std::string label;
        bool last_child;
        bool inlined;

        plaintext_dumper(std::ostream& out)
            : state(out, ""), label(""), last_child(true),
              inlined(false) {}

        plaintext_dumper(plaintext_dumper_state state,
                         std::string label, bool last_child,
                         bool inlined)
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
                state.out << std::endl;
                state.out << state.prefix;
                state.out << (last_child ? '`' : '|');
                state.out << "- ";
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
            state.out << text;
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
            state.out << std::addressof(obj);
        }

        template <typename T>
        void cType(T const& obj) {
            // alternatively use typeid(T).name() which is not
            // required to be human readable
            state.out << boost::typeindex::type_id<T>().pretty_name();
        }

        template <typename T>
        void cTypeAddr(T const& obj) {
            state.out << ' ';
            cType(obj);
            state.out << ' ';
            cAddr(obj);
        }

        void cEnum(std::string const& text) {
            state.out << text;
        }

        void cValue(std::string const& text) {
            state.out << "'" << text << "'";
        }

        void cBlock(std::string const& text) {
            state.out << std::endl << state.prefix << "`- ";
            state.out << boost::replace_all_copy(text,
                                                 "\n",
                                                 "\n" + state.prefix + "  ");
        }

        void cErr(std::string const& text) {
            state.out << text;
        }

        template <typename T>
        void cErrTypeAddr(std::string const& text, T const& obj) {
            cErr(text);
            cTypeAddr(obj);
        }

        void cUndefined() {
            state.out << "undefined";
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
                    // Prefix content
                    + state.prefix
                    // Label indentation +2 for spaces around
                    + std::string(label.size()+2, ' ');
                state.out << boost::replace_all_copy(value,
                                                     "\n",
                                                     replacement);
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
        };

        template <typename T>
        void operator()(boost::spirit::x3::forward_ast<T> const& ast) {
            getDerived()(ast.get());
        };

        template <typename T, std::size_t N>
        void operator()(std::array<T, N> const& ops) {
            auto s = ops.size();
            cAttr<Derived>(ops, "size", s, false, true);
            for(auto const& op : ops) {
                cAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename T>
        void operator()(std::vector<T> const& ops) {
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

#endif //DUMP_PLAINTEXT_HPP
