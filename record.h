//
// Created by Anton Tcholakov on 2019-06-30.
//

#include <utility>
#include <iostream>
#include <memory>
#include <functional>
#include <tuple>
#include <utility>
#include "immer/map.hpp"

#include "atom.h"
#include "lens.h"
#include "tagged.h"

namespace aeternum {

    template<typename T>
    class field_name;

    class untyped_record;

    template<typename T>
    class field_name : public lens<tagged<untyped_record>, T> {
    public:
        using type = T;

        explicit field_name(const char *key_) noexcept;

        inline const atom key() const;

    private:
        const atom _field_key;
    };

    class untyped_record {
    public:
        using data = immer::map<atom, std::shared_ptr<void>>;
        using hasher = std::function<std::size_t(const untyped_record&)>;
        using equality_comparer = std::function<bool(const untyped_record&, const untyped_record&)>;

        untyped_record(data &&data, hasher hasher, equality_comparer equality_comparer);

        untyped_record(untyped_record &&base) noexcept;

        untyped_record(const untyped_record &base) = default;

        untyped_record& operator=(const untyped_record& other) = default;
        untyped_record& operator=(untyped_record&& other) = default;

        data raw_data() const;

        std::size_t get_hash() const { return _hasher(*this); }

        template<typename T>
        inline const std::shared_ptr<const T> get_ptr(const field_name<T> &field_name) const;

        template<typename T>
        inline const T get(const field_name<T> &field_name) const;

        template<typename T>
        inline untyped_record set(const field_name<T> &field_name, T &&value) const;

        template<typename T>
        inline untyped_record set(const field_name <T> &field_name, const std::shared_ptr<const T> &value) const;

        template<typename T>
        inline const T operator[](const field_name<T> &field_name) const;

    protected:
        data _data;
        hasher _hasher;
        equality_comparer _equality_comparer;

        friend bool operator==(const untyped_record& lhs, const untyped_record& rhs);
    };

    template<typename ...TFieldTypes>
    struct record_utils
    {
    public:
        template<const field_name<TFieldTypes>& ...names>
        struct hasher {
            std::size_t operator()(const untyped_record& record) {
                return get_hash<sizeof...(TFieldTypes) - 1, names...>(record);
            }
        };

        template<const field_name<TFieldTypes>& ...names>
        struct equality_comparer {
            bool operator()(const untyped_record& lhs, const untyped_record& rhs) {
                return equal_to<sizeof...(TFieldTypes) - 1, names...>(lhs, rhs);
            }
        };

    private:
        template<std::size_t N, const field_name<TFieldTypes>& ...names>
        static std::size_t get_hash(const untyped_record& record) {
            return N > 0
                   ? std::hash<typename std::tuple_element<std::greater<>{}(N, 1) ? N : 0, std::tuple<TFieldTypes...>>::type>{}(
                            record.get(std::get<N>(std::forward_as_tuple(names...))))
                     ^ (record_utils<TFieldTypes...>::get_hash<std::greater<>{}(N, 1) ? N : 0, names...>(record) << 1)
                   : std::hash<typename std::tuple_element<0, std::tuple<TFieldTypes...>>::type>{}(
                            record.get(std::get<0>(std::forward_as_tuple(names...))));
        }

        template<std::size_t N, const field_name<TFieldTypes>& ...names>
        static std::size_t equal_to(const untyped_record& lhs, const untyped_record& rhs) {
            return N > 0
                   ? std::equal_to<typename std::tuple_element<std::greater<>{}(N, 1) ? N : 0, std::tuple<TFieldTypes...>>::type>{}(
                            lhs.get(std::get<N>(std::forward_as_tuple(names...))), rhs.get(std::get<N>(std::forward_as_tuple(names...))))
                     && (record_utils<TFieldTypes...>::equal_to<std::greater<>{}(N, 1) ? N : 0, names...>(lhs, rhs) << 1)
                   : std::equal_to<typename std::tuple_element<0, std::tuple<TFieldTypes...>>::type>{}(
                            lhs.get(std::get<0>(std::forward_as_tuple(names...))), rhs.get(std::get<0>(std::forward_as_tuple(names...))));
        }
    };

    template<typename ...TFieldTypes>
    class fields {
    public:
        template<const atom& tag, const field_name<TFieldTypes> &...names>
        class record : public untyped_record {
        public:
            using tagged = tagged<record<tag, names...>>;

            static tagged make(TFieldTypes &&...fields) {
                return make_tagged(tag, record(std::forward<TFieldTypes>(fields)...));
            }

            explicit record(TFieldTypes &&...fields);

            explicit record(const std::shared_ptr<TFieldTypes> &...fields);

            explicit record(untyped_record &&base);

            record(const record &record) = default;

            record(record &&record) noexcept : untyped_record(std::move(record.raw_data()), _hasher, _equality_comparer) {}

        private:
            class builder {
            public:
                inline data build(TFieldTypes &&...fields) const;

                inline data build(const std::shared_ptr<TFieldTypes> &...fields) const;

            private:
                template<size_t N, typename TField>
                inline data build_inner(data &&data, TField &&field) const;

                template<size_t N, typename TField>
                inline data build_inner(data &&data, const std::shared_ptr<TField> &field) const;

                template<size_t N, typename TField, typename ...TFields>
                inline data build_inner(data &&data, TField &&field, TFields &&...fields) const;

                template<size_t N, typename TField, typename ...TFields>
                inline data build_inner(data &&data, const std::shared_ptr<TField> &field,
                                        const std::shared_ptr<TFields> &...fields) const;
            };

            static const typename record<tag, names...>::builder _builder;
            static const typename record_utils<TFieldTypes...>::template hasher<names...> _hasher;
            static const typename record_utils<TFieldTypes...>::template equality_comparer<names...> _equality_comparer;
        };
    };

// --------------------------------------------------------------------------------------------
//                        IMPLEMENTATION : LENS OPERATORS
// --------------------------------------------------------------------------------------------

    template<typename TA, typename TB, typename TC>
    inline lens<TA, TC> operator>>(const lens<TA, TB> &left, const lens<tagged<untyped_record>, TC> &right) {
        return lens<TA, TC>(
                [=](const TA &record) { return right.get(*left.get(record)); },
                [=](const TA &record, const std::shared_ptr<const TC> &value) {
                    return left.set(record, TB(std::move(right.set(*left.get(record), value))));
                });
    }

    template<typename TRecord>
    inline TRecord operator>>(const TRecord &record, const setter<tagged<untyped_record>> &setter) {
        return TRecord(setter.apply(static_cast<const tagged<untyped_record> &>(record)));
    }

// --------------------------------------------------------------------------------------------
//                           IMPLEMENTATION : FIELD NAME
// --------------------------------------------------------------------------------------------

    template<typename T>
    field_name<T>::field_name(const char *key_) noexcept
            : _field_key(aeternum::atom(key_)),
              lens<tagged<untyped_record>, T>(
                      [&](const tagged<untyped_record> &record) { return (*record).get_ptr(*this); },
                      [&](const tagged<untyped_record> &record, const std::shared_ptr<const T> &value) {
                          return make_tagged(record.get_tag(), (*record).set(*this, value));
                      }) {}

    template<typename T>
    const atom field_name<T>::key() const { return _field_key; }

// --------------------------------------------------------------------------------------------
//                           IMPLEMENTATION : RECORD
// --------------------------------------------------------------------------------------------

    untyped_record::untyped_record(untyped_record::data &&data, untyped_record::hasher hasher, untyped_record::equality_comparer equality_comparer)
            : _data(std::move(data)), _hasher(std::move(hasher)), _equality_comparer(std::move(equality_comparer)) {}

    untyped_record::data untyped_record::raw_data() const { return _data; }

    template<typename T>
    const T untyped_record::get(const field_name <T> &field_name) const {
        return *std::const_pointer_cast<const T>(std::static_pointer_cast<T>(_data[field_name.key()]));
    }

    template<typename T>
    const std::shared_ptr<const T> untyped_record::get_ptr(const field_name <T> &field_name) const {
        auto test = _data[field_name.key()];
        return std::const_pointer_cast<const T>(std::static_pointer_cast<T>(test));
    }

    template<typename T>
    untyped_record untyped_record::set(const field_name <T> &field_name, T &&value) const {
        return untyped_record(_data.set(field_name.key(), std::make_shared<T>(value)), _hasher, _equality_comparer);
    }

    template<typename T>
    untyped_record untyped_record::set(const field_name <T> &field_name, const std::shared_ptr<const T> &value) const {
        return untyped_record(
                _data.set(field_name.key(), std::static_pointer_cast<void>(std::const_pointer_cast<T>(value))), _hasher, _equality_comparer);
    }

    template<typename T>
    const T untyped_record::operator[](const field_name <T> &field_name) const {
        return get<T>(field_name);
    }

    untyped_record::untyped_record(untyped_record &&base) noexcept : _data(std::move(base._data)) {}

    bool operator==(const untyped_record &lhs, const untyped_record &rhs) {
        return lhs._equality_comparer(lhs, rhs);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    template<size_t N, typename TField, typename... TFields>
    untyped_record::data fields<TFieldTypes...>::record<tag, names...>::builder::build_inner(
            untyped_record::data &&data, const std::shared_ptr<TField> &field,
            const std::shared_ptr<TFields> &...fields) const {
        return build_inner<N + 1>(data.set(std::get<N>(std::forward_as_tuple(names...)).key(), field), fields...);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    template<size_t N, typename TField, typename... TFields>
    untyped_record::data fields<TFieldTypes...>::record<tag, names...>::builder::build_inner(
            untyped_record::data &&data, TField &&field, TFields &&... fields) const {
        return build_inner<N + 1>(
                data.set(std::get<N>(std::forward_as_tuple(names...)).key(), std::make_shared<TField>(field)),
                std::forward<TFields>(fields)...);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    template<size_t N, typename TField>
    untyped_record::data fields<TFieldTypes...>::record<tag, names...>::builder::build_inner(
            untyped_record::data &&data, const std::shared_ptr<TField> &field) const {
        return data.set(std::get<N>(std::forward_as_tuple(names...)).key(), field);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    template<size_t N, typename TField>
    untyped_record::data
    fields<TFieldTypes...>::record<tag, names...>::builder::build_inner(untyped_record::data &&data, TField &&field) const {
        return data.set(std::get<N>(std::forward_as_tuple(names...)).key(),
                        std::make_shared<TField>(std::forward<TField>(field)));
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    untyped_record::data
    fields<TFieldTypes...>::record<tag, names...>::builder::build(const std::shared_ptr<TFieldTypes> &... fields) const {
        return build_inner<0, TFieldTypes...>(data(), fields...);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    untyped_record::data fields<TFieldTypes...>::record<tag, names...>::builder::build(TFieldTypes &&... fields) const {
        return build_inner<0, TFieldTypes...>(data(), std::forward<TFieldTypes>(fields)...);
    }

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    const typename fields<TFieldTypes...>::template record<tag, names...>::builder fields<TFieldTypes...>::record<tag, names...>::_builder =
            fields<TFieldTypes...>::template record<tag, names...>::builder();

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    const typename record_utils<TFieldTypes...>::template hasher<names...> fields<TFieldTypes...>::record<tag, names...>::_hasher;

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    const typename record_utils<TFieldTypes...>::template equality_comparer<names...> fields<TFieldTypes...>::record<tag, names...>::_equality_comparer ;

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    fields<TFieldTypes...>::record<tag, names...>::record(TFieldTypes &&... fields)
            : untyped_record(_builder.build(std::forward<TFieldTypes>(fields)...), _hasher, _equality_comparer) {}

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    fields<TFieldTypes...>::record<tag, names...>::record(const std::shared_ptr<TFieldTypes> &... fields)
            : untyped_record(_builder.build(fields...), _hasher, _equality_comparer) {}

    template<typename... TFieldTypes>
    template<const atom& tag, const field_name<TFieldTypes> &... names>
    fields<TFieldTypes...>::record<tag, names...>::record(untyped_record &&base) : untyped_record(std::move(base)) {}
}

namespace std {

    template<>
    struct hash<aeternum::untyped_record>
    {
        typedef aeternum::untyped_record argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type& record) const noexcept
        {
            return record.get_hash();
        }
    };

}