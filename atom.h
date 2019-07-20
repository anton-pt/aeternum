//
// Created by Anton Tcholakov on 2019-06-30.
//

#pragma once

#include <utility>

#include "crc32.h"

namespace aeternum {
    struct atom
    {
        explicit constexpr atom(const char* str) noexcept
                : name(str),
                  hash(crc32(name, strlen_c(name))) { }

        atom(const atom& other) = default;
        atom(atom&& other) = default;
        atom& operator=(const atom& other) = default;
        atom& operator=(atom&& other) = default;

        constexpr const char* get_name() const {
            return name;
        }

        const char* name;
        size_t hash;
    };

    bool operator==(const atom& lhs, const atom& rhs)
    {
        return lhs.hash == rhs.hash
               && strcmp(lhs.name, rhs.name) == 0;
    }

    bool operator<(const atom& lhs, const atom& rhs)
    {
        return strcmp(lhs.name, rhs.name) < 0;
    }
}

namespace std {
    template<>
    struct hash<aeternum::atom>
    {
        typedef aeternum::atom argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& atom) const noexcept
        {
            return atom.hash;
        }
    };
}