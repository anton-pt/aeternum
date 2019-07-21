//
// Created by Anton Tcholakov on 2019-06-30.
//

#pragma once

#include <functional>

namespace aeternum {
    template<typename TRecord>
    class setter {
    public:
        explicit setter(std::function<TRecord(const TRecord &)> set);

        inline TRecord apply(const TRecord &record) const;

    private:
        const std::function<TRecord(const TRecord &)> _set;
    };

    template<typename TRecord, typename TField>
    class lens {
    public:
        using result_type = TField;

        lens(
                std::function<std::shared_ptr<const TField>(const TRecord &)> get,
                std::function<TRecord(const TRecord &, const std::shared_ptr<const TField> &)> set);

        inline std::shared_ptr<const TField> get(const TRecord &record) const;

        inline TRecord set(const TRecord &record, const std::shared_ptr<const TField> &value) const;

        inline TRecord set(const TRecord &record, TField &&value) const;

        inline const setter<TRecord> set(const std::shared_ptr<const TField> &value) const;

        inline const setter<TRecord> set(TField &&value) const;

    private:
        const std::function<std::shared_ptr<const TField>(const TRecord &)> _get;
        const std::function<TRecord(const TRecord &, std::shared_ptr<const TField>)> _set;
    };


    template<typename TRecord>
    setter<TRecord>::setter(std::function<TRecord(const TRecord &)> set) : _set(std::move(set)) {}

    template<typename TRecord>
    TRecord setter<TRecord>::apply(const TRecord &record) const { return _set(record); }

    template<typename TRecord, typename TField>
    lens<TRecord, TField>::lens(
            std::function<std::shared_ptr<const TField>(const TRecord &)> get,
            std::function<TRecord(const TRecord &, const std::shared_ptr<const TField> &)> set)
            : _get(std::move(get)), _set(std::move(set)) {}

    template<typename TRecord, typename TField>
    std::shared_ptr<const TField> lens<TRecord, TField>::get(const TRecord &record) const { return _get(record); }

    template<typename TRecord, typename TField>
    TRecord lens<TRecord, TField>::set(const TRecord &record, const std::shared_ptr<const TField> &value) const {
        return _set(record, value);
    }

    template<typename TRecord, typename TField>
    TRecord lens<TRecord, TField>::set(const TRecord &record, TField &&value) const {
        return _set(record, std::make_shared<TField>(std::forward<TField>(value)));
    }

    template<typename TRecord, typename TField>
    const setter<TRecord> lens<TRecord, TField>::set(const std::shared_ptr<const TField> &value) const {
        return setter<TRecord>([&](const TRecord &record) { return set(record, value); });
    }

    template<typename TRecord, typename TField>
    const setter<TRecord> lens<TRecord, TField>::set(TField &&value) const {
        return setter<TRecord>([&](const TRecord &record) { return set(record, std::forward<TField>(value)); });
    }

    template<typename TA, typename TB, typename TC>
    inline lens<TA, TC> operator>>(const lens<TA, TB> &left, const lens<TB, TC> &right) {
        return lens<TA, TC>(
                [=](TA record) { return right.get(left.get(record)); },
                [=](TA record, const std::shared_ptr<TC> &value) {
                    return left.set(record, right.set(left.get(record), value));
                });
    }
}