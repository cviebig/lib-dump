#ifndef DUMP_FOREST_HPP
#define DUMP_FOREST_HPP

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
     * Struct that keeps the state of the forest_dumper.
     */
    struct forest_state {
        std::ostream& out;
        unsigned int level;

        forest_state(std::ostream& out)
            : out(out), level(0) {}
    };

    /**
     * Base class for data structure dumpers.
     *
     * We use the Curiously recurring template pattern here to be able
     * to call method's of the derived class.
     */
    template <typename Derived>
    struct forest_dumper {
        typedef forest_dumper<Derived> Self;

        const std::string black = "\\textcolor{black}{";
        const std::string red = "\\textcolor{red}{";
        const std::string green = "\\textcolor{green}{";
        const std::string yellow = "\\textcolor{orange}{";
        const std::string blue = "\\textcolor{blue}{";
        const std::string magenta = "\\textcolor{magenta}{";
        const std::string cyan = "\\textcolor{cyan}{";
        const std::string white = "\\textcolor{white}{";

        const std::string violet = "\\textcolor{violet}{";

        const std::string bold = "\\textbf{";

        const std::string reset = "}";

        forest_state state;
        std::string label;
        bool last_child;
        bool inlined;

        forest_dumper(std::ostream& out)
            : state(out), label(""), last_child(true),
              inlined(false) {
            print_prefix();
        }

        forest_dumper(forest_state state, std::string label,
                      bool last_child, bool inlined)
            : state(state), label(label), last_child(last_child),
              inlined(inlined) {
            this->state.level++;
            print_prefix();
        }

        ~forest_dumper() {
            print_postfix();
        }

        Derived &getDerived() {
            return static_cast<Derived&>(*this);
        }

        const Derived &getDerived() const {
            return static_cast<const Derived&>(*this);
        }

        inline
        std::string escape(std::string text) {
            boost::replace_all(text, "_", "\\_");
            boost::replace_all(text, "&", "\\&");
            boost::replace_all(text, "#", "\\#");
            /* Commas are replaced because of their syntactical meaning in LaTeX
             * forest package as they are used to configure additional settings
             * inside a node or subtree.
             */
            boost::replace_all(text, "{", "\\{");
            boost::replace_all(text, "}", "\\}");
            boost::replace_all(text, ",", "{,}");
            boost::replace_all(text, "[", "{[}");
            boost::replace_all(text, "]", "{]}");
            boost::replace_all(text, "=", "{=}");
            return text;
        }

        void print_prefix() {
            if(state.level == 0) {
                state.out << "\\begin{forest}" << std::endl;
                state.out << "  for tree={" << std::endl;
                state.out << "    font=\\ttfamily," << std::endl;
                state.out << "    grow'=0," << std::endl;
                state.out << "    child anchor=west," << std::endl;
                state.out << "    parent anchor=south," << std::endl;
                state.out << "    anchor=west," << std::endl;
                state.out << "    calign=first," << std::endl;
                state.out << "    edge path={" << std::endl;
                state.out << "      \\noexpand\\path [draw, \\forestoption{edge}]" << std::endl;
                state.out << "      (!u.south west) +(7.5pt,0) |- node[fill,inner sep=1.25pt] {} (.child anchor)\\forestoption{edge label};" << std::endl;
                state.out << "    }," << std::endl;
                state.out << "    before typesetting nodes={" << std::endl;
                state.out << "      if n=1" << std::endl;
                state.out << "        {insert before={[,phantom]}}" << std::endl;
                state.out << "        {}" << std::endl;
                state.out << "    }," << std::endl;
                state.out << "    fit=band," << std::endl;
                state.out << "    before computing xy={l=15pt}," << std::endl;
                state.out << "  }" << std::endl;
            }
            if(!inlined) {
                state.out << std::string(state.level, ' ');
                state.out << "[";
            } else {
                state.out << ' ';
            }
            if(label.length() > 0) {
                state.out << escape(label) << ' ';
            }
        }

        void print_postfix() {
            if(!inlined) {
                state.out << "]";
                state.out << std::endl;
            } else {
                state.out << ' ';
            }
            if(state.level == 0) {
                state.out << "\\end{forest}" << std::endl;
            }
        }

        template <typename Node>
        void cNode(Node const& node, std::string const& text) {
            state.out << bold << magenta << escape(text) << reset << reset;
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
                      << escape(boost::typeindex::type_id<T>().pretty_name())
                      << reset;
        }

        template <typename T>
        void cTypeAddr(T const& obj) {
            cType(obj);
            cAddr(obj);
        }

        void cEnum(std::string const& text) {
            state.out << yellow << escape(text) << reset;
        }

        void cValue(std::string const& text) {
            if(text.find("\n") == std::string::npos) {
                state.out << green << " " << escape(text) << " " << reset;
            } else {
                cBlock(text);
            }
        }

        void cBlock(std::string const& text) {
            std::string t =
                boost::replace_all_copy(escape(text),
                                        "\n",
                                        "\\\\\n  ");
            boost::replace_all(t, " ", "\\ ");
            boost::replace_all(t, "/home/chris/projects/thesis/code/umbrella/", "");
            boost::replace_all(t, "lib-udf-clang-sdf-sdf-col/test/src/", "");
            state.out << "\\\\\n\\ \\ " << t << ", align=left";
        }

        void cErr(std::string const& text) {
            state.out << bold << red << escape(text) << reset;
        }

        template <typename T>
        void cErrTypeAddr(std::string const& text, T const& obj) {
            cErr(text);
            cTypeAddr(obj);
        }

        void cUndefined() {
            state.out << violet << "undefined" << reset;
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
            // TODO: Support multi line strings as in `console_dumper`
            cValue(value);
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

#endif //DUMP_FOREST_HPP
