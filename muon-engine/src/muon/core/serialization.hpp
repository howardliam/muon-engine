#pragma once

#include "toml++/toml.hpp"

#include <concepts>
#include <expected>

namespace muon {

using SerializationResult = toml::table;

template <class T>
concept Serializable = requires(const T &t) {
    { T::serialize(t) } -> std::same_as<SerializationResult>;
};

enum class DeserializationError {
    FieldNotPresent,
};

template <class T>
using DeserializationResult = std::expected<T, DeserializationError>;

template <class T>
concept Deserializable = requires(T &t, const toml::table &table) {
    { T::deserialize(table) } -> std::same_as<DeserializationResult<T>>;
};

} // namespace muon
