#include "dump/tikz.hpp"
#include "dump/latex_support.hpp"
#include "dump/demo/dumper.hpp"
#include "test.hpp"

#include <fstream>

namespace dump {

    TEST_CASE( "Dummy test case for tikz", "[dumper]" ) {
        demo::tire t{"Tire Brand", 215, 16};
        demo::car c{"Car Brand", "Model 2000", 2002,
                    {demo::fuel_type::electric, 69}, {t, t, t, t}};

        std::ofstream full{"tikz.tex"};
        full << ::dump::latex_support{}.get_header();
        demo::dumper<tikz_dumper>{full}(c);
        full << ::dump::latex_support{}.get_footer();
        full << std::endl;

        std::ofstream sparse{"tikz_sparse.tex"};
        sparse << ::dump::latex_support{}.get_header();
        demo::dumper<tikz_dumper, ::dump::dispatch::sparse>{sparse}(c);
        sparse << ::dump::latex_support{}.get_footer();
        sparse << std::endl;

        CHECK( true );
    }

}
