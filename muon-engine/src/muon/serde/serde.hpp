#pragma once

#include <concepts>
#include <expected>

namespace muon::serde {

template <typename Data, typename To>
concept Serializable = requires(const Data &data) {
    { data.serialize() } -> std::same_as<To>;
};

template <typename Data, typename From, typename Error>
concept Deserializable = requires(Data &data, const From &from) {
    { Data::deserialize(from) } -> std::same_as<std::expected<Data, Error>>;
};

}
