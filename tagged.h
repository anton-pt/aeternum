//
// Created by Anton Tcholakov on 2019-07-20.
//

#pragma once

#include <functional>
#include <memory>
#include <set>

#include "atom.h"

namespace aeternum {

    template<typename T>
    class tagged;

    class tagged_untyped {
    public:

        virtual ~tagged_untyped() = default;

        template<typename T, typename Hash = std::hash<T>, typename Equals = std::equal_to<T>>
        tagged_untyped(atom tag, std::shared_ptr<T> data);

        tagged_untyped(const tagged_untyped& other) = default;
        tagged_untyped(tagged_untyped&& other) noexcept = default;

        tagged_untyped& operator=(const tagged_untyped& other) = default;
        tagged_untyped& operator=(tagged_untyped&& other) = default;

        template<const atom& tag, typename T>
        const tagged<T> match() const;

        const atom get_tag() const { return _tag; }

        operator bool() const { return _data != nullptr; }

        std::size_t get_hash() const;

        bool operator==(const tagged_untyped& rhs) const {
            return get_tag() == rhs.get_tag()
                   && _equality_comparer(_data, rhs._data);
        }

    protected:
        atom _tag;
        std::shared_ptr<void> _data;
        std::function<std::size_t(const std::shared_ptr<void>&)> _hasher;
        std::function<bool(const std::shared_ptr<void>&, const std::shared_ptr<void>&)> _equality_comparer;
    };

    template<typename T, typename Hash, typename Equals>
    tagged_untyped::tagged_untyped(atom tag, std::shared_ptr<T> data)
            : _tag(tag),
              _data(std::static_pointer_cast<void>(data)),
              _hasher([](const std::shared_ptr<void>& data) { return Hash{}(*(std::static_pointer_cast<T>(data))); }),
              _equality_comparer([](const std::shared_ptr<void>& lhs, const std::shared_ptr<void>& rhs) { return lhs == rhs ? true : Equals{}(*std::static_pointer_cast<T>(lhs), *std::static_pointer_cast<T>(rhs)); }) { }

    template<typename T>
    class tagged : public tagged_untyped {
    public:
        tagged(atom tag, std::shared_ptr<T> data);

        tagged(const tagged& other) = default;
        tagged(tagged&& other) noexcept = default;

        tagged& operator=(const tagged& other) = default;
        tagged& operator=(tagged&& other) noexcept = default;

        T& operator*() const;
        T* operator->() const;

        template<typename U>
        operator tagged<U>() const { return tagged<U>(_tag, std::static_pointer_cast<U>(_data)); }

        template<typename Idx>
        const typename Idx::result_type operator[](const Idx& idx) const {
            return (*this)->operator[](idx);
        }
    };

    template<const atom& tag, typename T>
    const tagged<T> tagged_untyped::match() const {
        return _tag == tag
               ? tagged<T>(tag, std::static_pointer_cast<T>(_data))
               : tagged<T>(tag, nullptr);
    }

    std::size_t tagged_untyped::get_hash() const {
        std::size_t h1 = std::hash<aeternum::atom>{}(get_tag());
        std::size_t h2 = _hasher(_data);
        return h1 ^ (h2 << 1);
    }

    template<typename T>
    tagged<T>::tagged(atom tag, std::shared_ptr<T> data) : tagged_untyped(tag, data) { }

    template<typename T>
    T& tagged<T>::operator*() const {
        return *(std::static_pointer_cast<T>(_data));
    }

    template<typename T>
    T* tagged<T>::operator->() const {
        return std::static_pointer_cast<T>(_data).operator->();
    }

    template<typename T>
    tagged<T> make_tagged(atom tag, T&& data) {
        return tagged<T>(tag, std::make_shared<T>(std::forward<T>(data)));
    }

    template<typename T, class Compare = std::less<T>>
    bool operator<(const tagged<T>& lhs, const tagged<T>& rhs)
    {
        return lhs.get_tag() < rhs.get_tag()
               || (lhs.get_tag() == rhs.get_tag() && Compare(*lhs, *rhs));
    }
}

namespace std {
    template<>
    struct hash<aeternum::tagged_untyped>
    {
        typedef aeternum::tagged_untyped argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& tagged) const noexcept
        {
            return tagged.get_hash();
        }
    };

    template<typename T>
    struct hash<aeternum::tagged<T>>
    {
        typedef aeternum::tagged<T> argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& tagged) const noexcept
        {
            return tagged.get_hash();
        }
    };
}