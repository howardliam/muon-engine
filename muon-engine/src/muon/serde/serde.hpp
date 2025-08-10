#pragma once

#include <concepts>
#include <expected>

namespace muon::serde {

template <typename From, typename To>
auto serialize(const From &) -> To;

template <typename From, typename To>
concept serializable = requires(const From &from) {
    { serialize<From, To>(from) } -> std::same_as<To>;
};

template <typename From, typename To, typename Error>
auto deserialize(const From &) -> std::expected<To, Error>;

template <typename From, typename To, typename Error>
concept deserializable = requires(const From &from) {
    { deserialize<From, To, Error>(from) } -> std::same_as<std::expected<To, Error>>;
};

}
