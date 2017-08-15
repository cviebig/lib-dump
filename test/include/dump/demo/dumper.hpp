#ifndef DUMP_DEMO_DUMPER_HPP
#define DUMP_DEMO_DUMPER_HPP

#include "dump/demo/car.hpp"

namespace dump { namespace dispatch {
    struct full;
    struct sparse;
}}

namespace dump { namespace demo {

    template <template <typename> class Base,
              typename D = ::dump::dispatch::full>
    struct dumper
        : public Base<dumper<Base, D>>
    {
        typedef dumper<Base, D> Self;
        using Base<Self>::Base;
        using Base<Self>::operator();

        void operator()(fuel_type const& obj)
        {
            switch(obj) {
            case fuel_type::diesel:
                this->cEnum("diesel");
                break;
            case fuel_type::petrol:
                this->cEnum("petrol");
                break;
            case fuel_type::gas:
                this->cEnum("gas");
                break;
            case fuel_type::electric:
                this->cEnum("electric");
                break;
            default:
                this->cUndefined();
                break;
            }
        }

        void operator()(engine const& obj)
        {
            this->cNode(obj, "Engine");
            this->cAttr(obj, "fuel", obj.fuel, false, true);
            this->cAttr(obj, "power", obj.power, true, true);
        }

        void operator()(tire const& obj)
        {
            this->cNode(obj, "Tire");
            this->cAttr(obj, "manufacturer", obj.manufacturer, false, true);
            this->cAttr(obj, "width", obj.width, false, true);
            this->cAttr(obj, "size", obj.size, true, true);
        }

        void operator()(four_tires const& obj)
        {
            this->cList(obj, "Tires");
        }

        void operator()(car const& obj)
        {
            this->cNode(obj, "Car");
            this->cAttr(obj, "manufacturer", obj.manufacturer, false, true);
            this->cAttr(obj, "model", obj.model, false, true);
            this->cAttr(obj, "year", obj.year, false, true);
            this->cAttr(obj, "main engine", obj.main_engine, false);
            this->cAttr(obj, "tires", obj.tires, true);
        }

        void operator()(cars const& obj)
        {
            this->cList(obj, "Cars");
        }

    };

    template <template <typename> class Base>
    struct dumper<Base, ::dump::dispatch::sparse>
        : public Base<dumper<Base, ::dump::dispatch::sparse>>
    {
        typedef dumper<Base, ::dump::dispatch::sparse> Self;
        using Base<Self>::Base;
        using Base<Self>::operator();

        void operator()(fuel_type const& obj)
        {
            switch(obj) {
            case fuel_type::diesel:
                this->cEnum("diesel");
                break;
            case fuel_type::petrol:
                this->cEnum("petrol");
                break;
            case fuel_type::gas:
                this->cEnum("gas");
                break;
            case fuel_type::electric:
                this->cEnum("electric");
                break;
            default:
                this->cUndefined();
                break;
            }
        }

        void operator()(car const& obj)
        {
            this->cNode(obj, "Car");
            this->cAttr(obj, "model", obj.model, false, true);
            this->cAttr(obj, "fuel", obj.main_engine.fuel, false, true);
            this->cAttr(obj, "power", obj.main_engine.power, true, true);
        }

        void operator()(cars const& obj)
        {
            this->cList(obj, "Cars");
        }

    };

}}

#endif //DUMP_DEMO_DUMPER_HPP
