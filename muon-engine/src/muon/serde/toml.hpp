#pragma once

#include "muon/serde/serde.hpp"
#include "toml++/toml.hpp"

namespace muon::serde {

template <typename From>
concept toml_serializable = serializable<From, toml::table>;

enum class toml_deserialize_error {
    FieldNotPresent,
};

template <typename To>
concept toml_deserializable = deserializable<toml::table, To, toml_deserialize_error>;

}
