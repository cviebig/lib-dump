#ifndef DUMP_TIKZ_HPP
#define DUMP_TIKZ_HPP

#include "dump/support.hpp"
#include "dump/x3_support.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>

#include <array>
#include <cstdint> // uintptr_t
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace dump {

    /**
     * Struct that keeps the state of the console_dumper.
     */
    struct tikz_dumper_state {
        /**
         *
         */
        std::ostream& parent;

        /**
         *
         */
        std::ostringstream out;

        /**
         *
         */
        std::ostringstream eout;

        /**
         * Created nodes
         */
        address_set nodes;

        /**
         * Node aliases
         */
        address_map_ptr aliases;

        /**
         * Level within the nested hierarchy
         */
        unsigned int level;

        /**
         * Hints to be given to `cNode`
         */
        address_hints hints;

        tikz_dumper_state(std::ostream& parent)
            : parent(parent), out(std::ostringstream()), eout(std::ostringstream()),
              nodes(std::make_shared<std::set<object_address>>()),
              aliases(std::make_shared<address_map>()),
              level(0) {}

        tikz_dumper_state(std::ostream& parent, address_hints hints)
            : parent(parent), out(std::ostringstream()), eout(std::ostringstream()),
              nodes(std::make_shared<std::set<object_address>>()),
              aliases(std::make_shared<address_map>()),
              level(0), hints(hints) {}

        tikz_dumper_state(tikz_dumper_state& other)
            : parent(other.parent), out(std::ostringstream()), eout(std::ostringstream()),
              nodes(other.nodes), aliases(other.aliases),
              level(other.level), hints(other.hints) {}
    };

    /**
     * Base class for data structure dumpers.
     *
     * We use the Curiously recurring template pattern here to be able
     * to call method's of the derived class.
     */
    template <typename Derived>
    struct tikz_dumper {
        typedef tikz_dumper<Derived> Self;

        tikz_dumper_state state;
        bool inlined;
        std::shared_ptr<std::ostringstream> edges;
        std::vector<std::shared_ptr<std::ostringstream>> child_edges;

        bool create_node;
        boost::optional<object_address> node_id;
        uintptr_t node_address;
        std::string node_type;
        std::vector<std::pair<std::string,
                              boost::optional<std::string>>
                    > node_childs;

        const char* tikz_nodepart[25] = {
            "one", "two", "three", "four", "five", "six", "seven",
            "eight", "nine", "ten", "eleven", "twelve", "thirteen",
            "fourteen", "fifteen", "sixteen", "seventeen", "eighteen",
            "nineteen", "twenty", "twentyone", "twentytwo",
            "twentythree", "twentyfour", "twentyfive"};

        tikz_dumper(std::ostream& out, bool inlined = false)
            : state(out), inlined(inlined), edges(), create_node(false) {
            print_global_prefix();
        }

        tikz_dumper(std::ostream& out, address_hints hints,
                    bool inlined = false)
            : state(out, hints), inlined(inlined), edges(), create_node(false) {
            print_global_prefix();
        }

        tikz_dumper(tikz_dumper_state& state, bool inlined,
                    std::shared_ptr<std::ostringstream> edges)
            : state(state), inlined(inlined), edges(edges), create_node(false) {
            this->state.level++;
            print_global_prefix();
        }

        ~tikz_dumper() {
            for(auto const& child_e : child_edges) {
                if(child_e) {
                    state.eout << child_e->str();
                }
            }
            if(!inlined) {
                print_node();
                state.parent << state.out.str();
            }
            if(edges) {
                *edges.get() << state.eout.str();
            } else {
                state.parent << state.eout.str();
            }
            print_global_postfix();
        }

        Derived &getDerived() {
            return static_cast<Derived&>(*this);
        }

        const Derived &getDerived() const {
            return static_cast<const Derived&>(*this);
        }

        void print_global_prefix() {
            if(state.level==0) {
                state.parent << "\\begin{tikzpicture}";
                state.parent << "[";
                state.parent << "layered layout,";
                state.parent << "every edge/.style={";
                state.parent << "very thick, ";
                state.parent << "draw=blue!40!black!60, ";
                state.parent << "shorten >=1pt, shorten <=1pt}, ";
                state.parent << "every node/.style={";
                state.parent << "rectangle, ";
                state.parent << "text ragged, ";
                state.parent << "inner sep=2mm, ";
                state.parent << "rounded corners, ";
                state.parent << "shade, ";
                state.parent << "top color=white, ";
                state.parent << "bottom color=blue!50!black!20, ";
                state.parent << "draw=blue!40!black!60, ";
                state.parent << "very thick }";
                state.parent << "]";
                state.parent << std::endl;
            }
        }

        void print_global_postfix() {
            if(state.level==0) {
                state.parent << "\\end{tikzpicture}" << std::endl;
            }
        }

        inline
        std::string escape(std::string text) {
            boost::replace_all(text, "_", "\\_");
            boost::replace_all(text, "&", "\\&");
            boost::replace_all(text, "#", "\\#");
            boost::replace_all(text, "{", "\\{");
            boost::replace_all(text, "}", "\\}");
            boost::replace_all(text, "->", "$\\rightarrow$");
            boost::replace_all(text, "<=", "$\\leq$");
            boost::replace_all(text, ">=", "$\\geq$");
            boost::replace_all(text, "<", "$<$");
            boost::replace_all(text, ">", "$>$");
            return text;
        }

        void print_node() {
            if(create_node && !inlined) {
                if(!node_id) {
                    throw std::runtime_error(
                          "node_id not initialized in tikz_dumper");
                }
                state.out << "\\node"
                          << std::endl;
                // Style
                state.out << "\t"
                          << "["
                          << "rectangle split"
                          << ", rectangle split parts="
                          << 1+node_childs.size()
                          << ", text ragged";
                if(state.hints) {
                    auto const& hint = state.hints->find(node_id.get());
                    if(hint != state.hints->end()) {
                        state.out << ", " << hint->second;
                    }
                }
                state.out << "]"
                          << std::endl;
                // Identifier
                state.out << "\t"
                          << " ("
                          << node_id.get().str()
                          << ")"
                          << std::endl;
                // Content start
                state.out << "\t"
                          << "{"
                          << std::endl;
                // Content
                state.out << "\t\t"
                          << "\\textbf{"
                          << node_type
                          << "}"
                       /* if uncommented increase `i` and rectangle split parts
                        * by 1 */
                       // << std::endl
                       // << "\t\t"
                       // << "\\nodepart{two}"
                       // << std::endl
                       // << "\t\t"
                       // << "0x" << std::hex << node_address
                          << std::endl;
                unsigned i = 0;
                for(auto const& child : node_childs) {
                    if(child.second != boost::none) {
                        state.out << "\t\t"
                                  << "\\nodepart{"
                                  << ((i++<=25) ? tikz_nodepart[i] : "too many nodes")
                                  << "}"
                                  << std::endl
                                  << "\t\t"
                                  << "\\textit{"
                                  << escape(child.first)
                                  << "}"
                                  << std::endl
                                  << "\t\t"
                                  << child.second.get()
                                  << std::endl;
                    }
                }
                // Content end
                state.out << "\t"
                          << "};"
                          << std::endl;
            }
        }

        template <typename Node>
        void cNode(Node const& node, std::string const& text) {
            if(!inlined) {
                auto ret = state.nodes->emplace(node);
                if(ret.second && !inlined) {
                    node_id = object_address(node);
                    node_address = reinterpret_cast<uintptr_t>(std::addressof(node));
                    node_type = text;
                    create_node = true;
                }
            }
        }

        template <typename Parent, typename Child>
        void cDebug(Parent const& parent, Child const& child) {
            auto pT = boost::typeindex::type_id<Parent>().pretty_name();
            auto pC = boost::typeindex::type_id<Child>().pretty_name();
            boost::replace_all(pT, "_", "\\_");
            boost::replace_all(pC, "_", "\\_");
            boost::replace_all(pT, "&", "\\&");
            boost::replace_all(pC, "&", "\\&");

            auto const& parent_address = state.aliases->resolve(parent);
            auto const& child_address = state.aliases->resolve(child);

            state.out << "cAttr: parent "
                      << print_address(get_address(parent))
                      << " (" << parent_address.str() << ")"
                      << " [" << state.nodes->count(parent_address) << "]"
                      << " of type "
                      << pT
                      << ", child "
                      << print_address(get_address(child))
                      << " (" << child_address.str() << ")"
                      << " [" << state.nodes->count(child_address) << "] "
                      << " of type "
                      << pC
                      << std::endl;
        }

        inline
        std::string get_current_tikz_nodepart() {
            unsigned int i = 1+node_childs.size();
            return (i<=25) ? tikz_nodepart[i] : "too many nodes";
        }

        template <typename Dumper, typename Parent, typename Child>
        void cAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {

            if(inlined) {
                // Add alias if childs of the child should connect to the parent
                auto const& parent_address = state.aliases->resolve(parent);
                auto const& child_address = state.aliases->resolve(child);
                if(parent_address.str() != child_address.str()) {
                    /*
                    std::cout << "Adding alias from " << child_address.str()
                              << " to " << parent_address.str() << std::endl;
                    */
                    state.aliases->add(child_address, parent_address);
                } else {
                    /*
                    std::cout << "Did not add alias from "
                              << child_address.str() << " to "
                              << parent_address.str() << std::endl;
                    */
                }
            }

            auto child_e = std::make_shared<std::ostringstream>();
            child_edges.push_back(child_e);
            Dumper child_dumper(state, inlined, child_e);
            child_dumper(child);
            if(!inlined) {
                // node_childs.emplace_back(label, boost::none);
            } else {
                std::string child_out = child_dumper.state.out.str();
                if(child_out.size() > 0) {
                    node_childs.emplace_back(label, child_out);
                }
            }

            // cDebug(parent, child);

            auto const& parent_address = state.aliases->resolve(parent);
            auto const& child_address = state.aliases->resolve(child);

            if(!inlined &&
               state.nodes->count(parent_address) > 0 &&
               state.nodes->count(child_address) > 0) {
                // Only create edge if both nodes exist
                state.eout << "\\draw"
                           << " ("
                           << parent_address.str()
                    // FIX: Reference node part instead of full node.
                    // << "." << get_current_tikz_nodepart()
                           << ")"
                           << " edge[->]"
                           << " (" << child_address.str() << ")"
                           << ";"
                           << std::endl;
            }
        }

        template <typename Parent, typename Child>
        void cAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {
            cAttr<Derived>(parent, label, child, last_child, inlined);
        }

        template <typename Dumper, typename Parent, typename Child>
        void cListAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {
            if(inlined) {
                auto const& parent_address = state.aliases->resolve(parent);
                auto const& child_address = state.aliases->resolve(child);
                // Add alias if childs of the child should connect to the parent
                if(parent_address.str() != child_address.str()) {
                    /*
                    std::cout << "Adding alias from " << child_address.str()
                              << " to " << parent_address.str() << std::endl;
                    */
                    state.aliases->add(child_address, parent_address);
                } else {
                    /*
                    std::cout << "Did not add alias from "
                              << child_address.str() << " to "
                              << parent_address.str() << std::endl;
                    */
                }
            }

            auto child_e = std::make_shared<std::ostringstream>();
            child_edges.push_back(child_e);
            Dumper child_dumper(state, inlined, child_e);
            child_dumper(child);
            if(!inlined) {
                // list items are not listed inside the tikz node
            } else {
                node_childs.emplace_back(label, child_dumper.state.out.str());
            }

            // cDebug(parent, child);

            auto const& parent_address = state.aliases->resolve(parent);
            auto const& child_address = state.aliases->resolve(child);

            // Only create edge if both nodes exist
            if(!inlined
               && state.nodes->count(parent_address) > 0
               && state.nodes->count(child_address) > 0) {
                state.eout << "\\draw"
                           << " (" << parent_address.str() << ")"
                           << " edge[->]"
                           << " (" << child_address.str() << ")"
                           << ";"
                           << std::endl;
            }
        }

        template <typename Parent, typename Child>
        void cListAttr(Parent const& parent, std::string const& label,
                   Child const& child, bool last_child,
                   bool inlined = false) {
            cListAttr<Derived>(parent, label, child, last_child, inlined);
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
        }

        template <typename T>
        void cType(T const& obj) {
        }

        template <typename T>
        void cTypeAddr(T const& obj) {
        }

        void cEnum(std::string const& text) {
            state.out << escape(text);
        }

        void cValue(std::string const& text) {
            state.out << escape(text);
        }

        void cBlock(std::string const& text) {
            // omitted
        }

        void cErr(std::string const& text) {
            state.out << escape(text);
        }

        template <typename T>
        void cErrTypeAddr(std::string const& text, T const& obj) {
        }

        void cUndefined() {
            state.out << escape("undefined");
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
            cValue(value);
        }

        template <typename T>
        void operator()(std::shared_ptr<T> const& ast) {
            if(ast != nullptr) {
                state.aliases->add(ast);
                getDerived()(*ast);
            } else {
                cUndefined();
            }
        }

        template <typename... _Args>
        void operator()(boost::variant<_Args...> const& ast) {
            state.aliases->add(ast);
            boost::apply_visitor(getDerived(), ast);
        }

        template <typename T>
        void operator()(boost::optional<T> const& ast) {
            if(ast) {
                state.aliases->add(ast);
                getDerived()(ast.get());
            } else {
                cUndefined();
            }
        }

        // Boost Spirit X3 variant type
        template <typename T,
              typename std::enable_if<is_variant<T>::value, bool>::type = false>
        void operator()(T const& ast) {
            state.aliases->add(ast);
            boost::apply_visitor(getDerived(), ast);
        }

        template <typename T>
        void operator()(boost::spirit::x3::forward_ast<T> const& ast) {
            state.aliases->add(ast);
            getDerived()(ast.get());
        }

        template <typename T, std::size_t N>
        void operator()(std::array<T, N> const& ops) {
            auto s = ops.size();
            for(auto const& op : ops) {
                cListAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename T>
        void operator()(std::vector<T> const& ops) {
          auto s = ops.size();
          for(auto const& op : ops) {
              cListAttr<Derived>(ops, "item", op, --s==0);
          }
        }

        template <typename Key, typename Compare, typename Allocator>
        void operator()(std::set<Key, Compare, Allocator> const& ops) {
            auto s = ops.size();
            for(auto const& op : ops) {
                cListAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename Key, typename T, typename Compare, typename Allocator>
        void operator()(std::map<Key, T, Compare, Allocator> const& ops) {
            auto s = ops.size();
            for(auto const& op : ops) {
                cListAttr<Derived>(ops, "item", op, --s==0);
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
          for(auto const& op : ops) {
              cListAttr<Derived>(ops, "item", op, --s==0);
          }
        }

        template <typename T, std::size_t N, typename Allocator>
        void operator()(boost::container::small_vector<T, N, Allocator> const& ops) {
            auto s = ops.size();
            for(auto const& op : ops) {
                cListAttr<Derived>(ops, "item", op, --s==0);
            }
        }

        template <typename T,
              typename std::enable_if<!is_variant<T>::value, bool>::type = false>
        void operator()(T const& ast) {
            cErrTypeAddr("not implemented", ast);
        }
    };

}

#endif //DUMP_TIKZ_HPP
