#ifndef DUMP_FILESYSTEM_HPP
#define DUMP_FILESYSTEM_HPP

#include "dump/console.hpp"
#include "dump/forest.hpp"
#include "dump/latex_support.hpp"
#include "dump/plaintext.hpp"
#include "dump/tikz.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <fstream>

namespace dump { namespace dispatch {
    struct full;
}}

namespace dump {

    template <template <template <typename> class, typename> class Dumper,
              typename D = ::dump::dispatch::full,
              bool stdcerr = false,
              typename T>
    void fs(std::string const& project,
            std::string const& artifact,
            std::string const& type,
            T const& obj,
            bool inlined = false) {

        // TODO: Turn into `static_if` when available
        if(stdcerr) {
            std::cerr << artifact << " " << type << std::endl;
            Dumper<::dump::console_dumper, D>{std::cerr}(obj);
            std::cerr << std::endl << std::endl;
        }

        auto consolef = boost::filesystem::temp_directory_path() / (project + "_" + artifact + "_" + type + ".ansi");
        boost::filesystem::ofstream consolefs{consolef};
        Dumper<::dump::console_dumper, D>{consolefs}(obj);
        consolefs << std::endl;

        auto plainf = boost::filesystem::temp_directory_path() / (project + "_" + artifact + "_" + type + ".txt");
        boost::filesystem::ofstream plainfs{plainf};
        Dumper<::dump::plaintext_dumper, D>{plainfs}(obj);
        plainfs << std::endl;

        auto tikzf = boost::filesystem::temp_directory_path() / (project + "_" + artifact + "_" + type + ".tikz.tex");
        boost::filesystem::ofstream tikzfs{tikzf};
        tikzfs << ::dump::latex_support{}.get_header();
        Dumper<::dump::tikz_dumper, D>{tikzfs, inlined}(obj);
        tikzfs << ::dump::latex_support{}.get_footer();
        tikzfs << std::endl;

        auto forestf = boost::filesystem::temp_directory_path() / (project + "_" + artifact + "_" + type + ".forest.tex");
        boost::filesystem::ofstream forestfs{forestf};
        forestfs << ::dump::latex_support{}.get_header();
        Dumper<::dump::forest_dumper, D>{forestfs}(obj);
        forestfs << ::dump::latex_support{}.get_footer();
        forestfs << std::endl;
    }

}

#endif //DUMP_FILESYSTEM_HPP
