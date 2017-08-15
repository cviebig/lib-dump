#include "dump/plaintext.hpp"
#include "dump/demo/dumper.hpp"
#include "test.hpp"

#include <fstream>

namespace dump {

    TEST_CASE( "Dummy test case for plaintext", "[dumper]" ) {
        demo::tire t{"Tire Brand", 215, 16};
        demo::car c{"Car Brand", "Model 2000", 2002,
                    {demo::fuel_type::electric, 69}, {t, t, t, t}};

        std::ofstream full{"plain.txt"};
        demo::dumper<plaintext_dumper>{full}(c);
        full << std::endl;

        std::ofstream sparse{"plain_sparse.txt"};
        demo::dumper<plaintext_dumper, ::dump::dispatch::sparse>{sparse}(c);
        sparse << std::endl;

        CHECK( true );
    }

}
