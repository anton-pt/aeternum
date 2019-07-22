//
// Created by Anton Tcholakov on 2019-07-20.
//

#include <chrono>
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

namespace music {
    namespace song {
        constexpr aeternum::atom tag("song");

        const aeternum::field_name<std::string> name_("name");
        const aeternum::field_name<std::string> artist_("artist");
        const aeternum::field_name<uint16_t> duration_("duration");

        using record =
            aeternum::fields<std::string, std::string, uint16_t>
                ::record<tag, name_, artist_, duration_>;
    }

    namespace lyrics {
        constexpr aeternum::atom tag("lyrics");

        struct line {
            std::string text;
            uint16_t timestamp;

            bool operator==(const line& other) const { return text == other.text && timestamp == other.timestamp; }
        };

        const aeternum::field_name<immer::vector<line>> lines_("lines");
        const aeternum::field_name<std::string> author_("author");

        using record =
            aeternum::fields<immer::vector<line>, std::string>
                ::record<tag, lines_, author_>;
    }

    namespace metadata {
        const aeternum::field_name<music::lyrics::record::tagged> lyrics_("lyrics");
    }
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

    template<>
    struct hash<music::song::record> {
        typedef music::song::record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };

    template<>
    struct hash<music::lyrics::line> {
        typedef music::lyrics::line argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& line) const noexcept
        {
            std::size_t h1 = std::hash<std::string>{}(line.text);
            std::size_t h2 = std::hash<uint16_t>{}(line.timestamp);
            return h1 ^ (h2 << 1);
        }
    };

    template<>
    struct hash<music::lyrics::record> {
        typedef music::lyrics::record argument_type;
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


    auto const john = person::record::make("John Smith", 42, contact::record::make("67890", "j.smith@email.com"));
    auto const junior = john
            | person::name_.set("Johnny Junior")
            | person::age_.set(12);

    auto const person_email_ = person::contact_ >> contact::email_;
    auto const junior_new_email = junior
            | person_email_.set("junior@email.com");

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

    std::cout << std::endl;

    auto never_gonna = music::song::record::make(
        "Never Gonna Give You Up",
        "Rick Astley",
        183);

    never_gonna = never_gonna
            | music::metadata::lyrics_.set(
                music::lyrics::record::make(
                    immer::vector<music::lyrics::line> {
                        { "Never gonna give you up", 22 },
                        { "Never gonna let you down", 26 }
                    },
                    "rickroller89"));

    std::cout << "\"Never gonna\" contains fields: ";
    for (auto& kvp : never_gonna->raw_data())
    {
        std::cout << kvp.first.name << ", ";
    }

    return 0;
}