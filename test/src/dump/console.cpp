#include "dump/console.hpp"
#include "dump/demo/dumper.hpp"
#include "test.hpp"

#include <iostream>
#include <fstream>

namespace dump {

    TEST_CASE( "Dummy test case for console", "[dumper]" ) {
        demo::tire t{"Tire Brand", 215, 16};
        demo::car c{"Car Brand", "Model 2000", 2002,
                    {demo::fuel_type::electric, 69}, {t, t, t, t}};

        std::cout << std::endl << std::endl;

        demo::dumper<console_dumper>{std::cout}(c);
        std::cout << std::endl << std::endl;

        demo::dumper<console_dumper, ::dump::dispatch::sparse>{std::cout}(c);
        std::cout << std::endl << std::endl;

        std::ofstream full{"ansi.txt"};
        demo::dumper<console_dumper>{full}(c);
        full << std::endl;

        std::ofstream sparse{"ansi_sparse.txt"};
        demo::dumper<console_dumper, ::dump::dispatch::sparse>{sparse}(c);
        sparse << std::endl;

        CHECK( true );
    }

}
