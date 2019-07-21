//
// Created by Anton Tcholakov on 2019-07-21.
//

#pragma once

#include "immer/vector.hpp"

namespace std {
    template<typename T>
    struct hash<immer::vector<T>>
    {
        typedef immer::vector<T> argument_type;
        typedef std::size_t result_type;

        template<typename Hash = std::hash<T>>
        result_type operator()(const argument_type& vec) const noexcept
        {
            auto hasher = Hash{};
            std::size_t seed = vec.size();
            for (auto& x : vec)
            {
                seed ^= hasher(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
}

template<typename T, class Equals = std::equal_to<T>>
bool operator==(const immer::vector<T>& lhs, const immer::vector<T>& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (auto i = 0; i < lhs.size(); i++)
    {
        if (!Equals(lhs[i], rhs[i]))
        {
            return false;
        }
    }

    return true;
}