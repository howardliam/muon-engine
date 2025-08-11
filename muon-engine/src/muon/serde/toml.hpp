#pragma once

#include "muon/serde/serde.hpp"
#include "toml++/toml.hpp"

namespace muon::serde {

template <typename From>
concept TomlSerializable = Serializable<From, toml::table>;

enum class TomlDeserializeError {
    FieldNotPresent,
};

template <typename To>
concept TomlDeserializable = Deserializable<toml::table, To, TomlDeserializeError>;

}
