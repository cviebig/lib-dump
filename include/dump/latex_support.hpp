#ifndef DUMP_LATEX_SUPPORT_HPP
#define DUMP_LATEX_SUPPORT_HPP

#include <sstream>

namespace dump {

    struct latex_support {
        std::string get_header() const {
            std::ostringstream oss;
            oss << "\\documentclass{minimal}" << std::endl;

            oss << "\\usepackage{luatex85}" << std::endl;

            oss << "\\usepackage[T1]{fontenc}" << std::endl;
            oss << "\\usepackage[utf8]{inputenc}" << std::endl;
            oss << "\\usepackage{textcomp}" << std::endl;

            oss << "\\usepackage{color}" << std::endl;
            oss << "\\usepackage{listings}" << std::endl;

            oss << "\\usepackage{tikz}" << std::endl;

            oss << "\\usetikzlibrary{graphdrawing,graphs,shapes}" << std::endl;
            oss << "\\usegdlibrary{layered}" << std::endl;

            oss << "\\usepackage{forest}" << std::endl;

            oss << "\\usepackage[active,tightpage]{preview}" << std::endl;

            oss << "\\PreviewEnvironment{lstlisting}" << std::endl;
            oss << "\\PreviewEnvironment{tikzpicture}" << std::endl;
            oss << "\\PreviewEnvironment{forest}" << std::endl;

            oss << "\\usepackage[numbered]{bookmark}" << std::endl;

            oss << "\\begin{document}" << std::endl;
            return oss.str();
        }

        std::string get_footer() const {
            std::ostringstream oss;
            oss << "\\end{document}" << std::endl;
            return oss.str();
        }
    };

}

#endif //DUMP_LATEX_SUPPORT_HPP
