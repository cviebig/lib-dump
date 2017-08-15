#ifndef DUMP_DEMO_CAR_HPP
#define DUMP_DEMO_CAR_HPP

#include <array>
#include <string>
#include <vector>

namespace dump { namespace demo {

    enum class fuel_type { diesel, petrol, gas, electric };

    struct engine {
        /* Fuel engine consumes
         */
        fuel_type fuel;

        /* Engine power in kilowatts
         */
        float power;

        /* Engine
         */
        engine(fuel_type fuel, float power) : fuel(fuel), power(power) {}
    };

    struct tire {
        /* Manufacturer
         */
        std::string manufacturer;

        /* Width of the tire in milimeters
         */
        unsigned width;

        /* Diameter of the tire in inch
         */
        float size;

        /* Tire
         */
        tire(std::string manufacturer, unsigned width, float size)
            : manufacturer(manufacturer), width(width), size(size) {}
    };

    using four_tires = std::array<tire, 4>;

    struct car {
        /* Manufacturer
         */
        std::string manufacturer;

        /* Model
         */
        std::string model;

        /* Year the car was built
         */
        unsigned year;

        /* Engine
         */
        engine main_engine;

        /* Tires mounted
         */
        four_tires tires;

        /* Car
         */
        car(std::string manufacturer, std::string model, unsigned year,
            engine main_engine, four_tires tires)
            : manufacturer(manufacturer), model(model), year(year),
              main_engine(main_engine), tires(tires) {}
    };

    using cars = std::vector<car>;

}}

#endif //DUMP_DEMO_CAR_HPP
