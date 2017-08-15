#ifndef DUMP_SUPPORT_HPP
#define DUMP_SUPPORT_HPP

#include "dump/x3_support.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/optional.hpp>
#include <boost/type_index.hpp>
#include <boost/variant.hpp>

#include <cstdint> // uintptr_t
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <iostream>

namespace dump {

    struct object_address {
        uintptr_t address;
        std::string type;

        template <typename T>
        object_address(T const& obj) {
            address = reinterpret_cast<uintptr_t>(std::addressof(obj));
            type = boost::typeindex::type_id<T>().raw_name();
            boost::replace_all(type, "_", "");
            boost::replace_all(type, "&", "");
        }

        std::string str() const {
            std::ostringstream oss;
            oss << "0x" <<  std::hex << address;
            oss << "-" << type;
            return oss.str();
        }

        bool operator<(object_address const& other) const {
            if(address < other.address) {
                return true;
            } else if(address == other.address
                      && type < other.type) {
                return true;
            } else {
                return false;
            }
        }

        bool operator==(object_address const& other) const {
            return address == other.address
                && type == other.type;
        }

        bool operator!=(object_address const& other) const {
            return address != other.address
                || type != other.type;
        }
    };

    typedef std::shared_ptr<std::set<object_address>> address_set;

    using address_hints =
        std::shared_ptr<std::map<object_address, std::string>>;

    struct ptr_visitor {
        template <typename T>
        object_address operator()(std::shared_ptr<T> const& obj) {
            if(obj) {
                return object_address(*obj);
            } else {
                return object_address(obj);
            }
        }

        template <typename... _Args>
        object_address operator()(boost::variant<_Args...> const& obj) {
            return boost::apply_visitor(*this, obj);
        }

        template <typename T>
        object_address operator()(boost::optional<T> const& obj) {
            if(obj) {
                return object_address(obj.get());
            } else {
                return object_address(obj);
            }
        }

        template <typename T,
              typename std::enable_if<is_variant<T>::value, bool>::type = false>
        object_address operator()(T const& obj) {
            return boost::apply_visitor(*this, obj);
        }

        template <typename T>
        object_address operator()(boost::spirit::x3::forward_ast<T> const& obj) {
            return object_address(obj.get());
        }

        template <typename T,
              typename std::enable_if<!is_variant<T>::value, bool>::type = false>
        object_address operator()(T const& obj) const {
            return object_address(obj);
        }
    };

    struct address_map {
        std::map<object_address, object_address> addresses;

        boost::optional<object_address>
        resolve(object_address const& address) {
            auto const& alias = addresses.find(address);
            if(alias == addresses.end()) {
                return boost::none;
            } else {
                /*
                std::cout << "Resolved " << alias->first.str() << " to "
                          << alias->second.str() << std::endl;
                */
                boost::optional<object_address> const& nested_alias =
                    resolve(alias->second);
                if(nested_alias) {
                    return nested_alias.get();
                } else {
                    return alias->second;
                }
            }
        }

       template <typename T>
       object_address resolve(T const& obj) {
            object_address const& address
                = object_address(obj);
            boost::optional<object_address> const& alias
                = resolve(address);

            if(alias) {
                return alias.get();
            } else {
                return address;
            }
        }

        template <typename T>
        void add(T const& obj) {
            auto const& o_address = object_address(obj);
            auto const& a_address = ptr_visitor{}(obj);
            if(o_address != a_address) {
                addresses.emplace(o_address, a_address);
            } else {
                throw std::runtime_error("No alias found");
            }
        }

        void add(object_address const& from, object_address const& to) {
            addresses.emplace(from, to);
        }
    };

    typedef std::shared_ptr<address_map> address_map_ptr;

    struct type_id_visitor {
        template <typename T>
        std::string operator()(std::shared_ptr<T> const& obj) {
            if(obj) {
                return (*this)(*obj);
            } else {
                return boost::typeindex::type_id<std::shared_ptr<T>>().pretty_name();
            }
        }

        template <typename... _Args>
        std::string operator()(boost::variant<_Args...> const& obj) {
            return boost::apply_visitor(*this, obj);
        }

        template <typename T>
        std::string operator()(boost::optional<T> const& obj) {
            if(obj) {
                return (*this)(obj.get());
            } else {
                return boost::typeindex::type_id<boost::optional<T>>().pretty_name();
            }
        }

        template <typename T,
              typename std::enable_if<is_variant<T>::value, bool>::type = false>
        std::string operator()(T const& ast) {
            return boost::apply_visitor(*this, ast);
        }

        template <typename T>
        std::string operator()(boost::spirit::x3::forward_ast<T> const& ast) {
            return (*this)(ast.get());
        }

        template <typename T,
              typename std::enable_if<!is_variant<T>::value, bool>::type = false>
        std::string operator()(T const& obj) const {
            return boost::typeindex::type_id<T>().pretty_name();
        }
    };

}

#endif //DUMP_SUPPORT_HPP
