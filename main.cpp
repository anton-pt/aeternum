//
// Created by Anton Tcholakov on 2019-07-20.
//

#include <cmath>
#include <iostream>

#include "immer/set.hpp"
#include "immer/vector.hpp"

#include "atom.h"
#include "collection_utils.h"
#include "tagged.h"
#include "record.h"

constexpr aeternum::atom apples("apples");
constexpr aeternum::atom oranges("oranges");

int fruit_count(const aeternum::tagged_untyped& tagged)
{
    if (auto as = tagged.match<apples, int>())
    {
        return *as;
    }
    if (auto os = tagged.match<oranges, int>())
    {
        return *os;
    }

    return 0;
}

namespace contact {
    constexpr aeternum::atom tag("contact");

    const aeternum::field_name<std::string> telephone_("telephone");
    const aeternum::field_name<std::string> email_("email");

    using record =
        aeternum::fields<std::string, std::string>
            ::record<tag, telephone_, email_>;

}

namespace person {
    constexpr aeternum::atom tag("person");

    const aeternum::field_name<std::string> name_("name");
    const aeternum::field_name<uint8_t> age_("age");
    const aeternum::field_name<contact::record::tagged> contact_("contact");

    using record =
        aeternum::fields<std::string, uint8_t, contact::record::tagged>
            ::record<tag, name_, age_, contact_>;
}

namespace circle {
    constexpr aeternum::atom tag("circle");

    const aeternum::field_name<double> radius_("radius");

    using record =
        aeternum::fields<double>
            ::record<tag, radius_>;
}

namespace rectangle {
    constexpr aeternum::atom tag("rectangle");

    const aeternum::field_name<double> width_("width");
    const aeternum::field_name<double> height_("height");

    using record =
        aeternum::fields<double, double>
            ::record<tag, width_, height_>;
}

namespace std {
    template<>
    struct hash<contact::record> {
        typedef contact::record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };

    template<>
    struct hash<person::record> {
        typedef person::record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };

    template<>
    struct hash<circle::record> {
        typedef circle::record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };

    template<>
    struct hash<rectangle::record> {
        typedef rectangle::record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };
}

double area(const aeternum::tagged_untyped& shape) {

    if (auto circle = shape.match<circle::tag, circle::record>())
    {
        auto const r = circle[circle::radius_];
        return M_PI * r * r;
    }
    if (auto rectangle = shape.match<rectangle::tag, rectangle::record>())
    {
        return rectangle[rectangle::width_]
               * rectangle[rectangle::height_];
    }

    throw std::logic_error("Unknown shape");
}

int main()
{
    const auto four_apples = aeternum::make_tagged(apples, 4);
    const auto four_oranges = aeternum::make_tagged(oranges, 4);
    const auto five_apples = aeternum::make_tagged(apples, 5);
    const auto four_more_oranges = aeternum::make_tagged(oranges, 4);

    std::cout << "The hash of 4 apples is: " << std::hash<aeternum::tagged_untyped>{}(four_apples) << std::endl;
    std::cout << "The hash of 4 oranges is: " << std::hash<aeternum::tagged_untyped>{}(four_oranges) << std::endl;
    std::cout << "The hash of 5 apples is: " << std::hash<aeternum::tagged_untyped>{}(five_apples) << std::endl;
    std::cout << "The hash of 4 more oranges is: " << std::hash<aeternum::tagged_untyped>{}(four_more_oranges) << std::endl;

    std::cout << std::endl;

    std::cout << "Comparing 4 apples and 4 oranges: " << (four_apples == four_oranges) << std::endl;
    std::cout << "Comparing 4 apples and 5 apples: " << (four_apples == five_apples) << std::endl;
    std::cout << "Comparing 4 oranges and 4 more oranges: " << (four_oranges == four_more_oranges) << std::endl;

    std::cout << std::endl;

    immer::set<aeternum::tagged_untyped> fruit;
    fruit = fruit.insert(four_apples);
    fruit = fruit.insert(four_oranges);
    fruit = fruit.insert(five_apples);
    fruit = fruit.insert(four_more_oranges);

    for (auto& f : fruit)
    {
        std::cout << "Set contains " << fruit_count(f) << " " << f.get_tag().name << std::endl;
    }

    std::cout << std::endl;

    auto const jane_contact = contact::record::make("12345", "jane_bond@007.com");
    std::cout << "Jane's telephone is " << jane_contact[contact::telephone_]
              << " and her email is " << jane_contact[contact::email_] << std::endl;

    std::cout << std::endl;

    auto person_email_ = person::contact_ >> contact::email_;

    auto const john = person::record::make("John Smith", 42, contact::record::make("67890", "j.smith@email.com"));
    auto const junior = john >> person::name_.set("Johnny Junior") >> person::age_.set(12);
    auto const junior_new_email = junior >> person_email_.set("junior@email.com");

    std::cout << john[person::name_]
              << ", age: " << +john[person::age_]
              << ", email: " << john[person::contact_][contact::email_] << std::endl;
    std::cout << junior[person::name_]
              << ", age: " << +junior[person::age_]
              << ", email: " << junior[person::contact_][contact::email_] << std::endl;
    std::cout << junior_new_email[person::name_]
              << ", age: " << +junior_new_email[person::age_]
              << ", email: " << junior_new_email[person::contact_][contact::email_] << std::endl;

    std::cout << std::endl;

    immer::set<aeternum::tagged<aeternum::untyped_record>> shapes{};

    shapes = shapes.insert(rectangle::record::make(5.0, 7.0));
    shapes = shapes.insert(circle::record::make(3.0));

    for (auto& shape : shapes)
    {
        std::cout << "Shapes contains a " << shape.get_tag().name
                  << " of area: " << area(shape) << std::endl;
    }

    return 0;
}