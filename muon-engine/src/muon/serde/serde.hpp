#pragma once

#include <concepts>
#include <expected>

namespace muon::serde {

template <typename Data, typename To>
auto serialize(const Data &data) -> To;

template <typename Data, typename To>
concept Serializable = requires(const Data &data) {
    { serialize<Data, To>(data) } -> std::same_as<To>;
};

template <typename From, typename Data, typename Error>
auto deserialize(const From &from) -> std::expected<Data, Error>;

template <typename Data, typename From, typename Error>
concept Deserializable = requires(const From &from) {
    { deserialize<From, Data, Error>(from) } -> std::same_as<std::expected<Data, Error>>;
};

}
