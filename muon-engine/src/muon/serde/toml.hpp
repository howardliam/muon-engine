#pragma once

#include "muon/serde/serde.hpp"
#include "toml++/toml.hpp"

namespace muon::serde {

template <typename Data>
concept TomlSerializable = Serializable<Data, toml::table>;

enum class TomlDeserializeError {
    FieldNotPresent,
};

template <typename Data>
concept TomlDeserializable = Deserializable<Data, toml::table, TomlDeserializeError>;

}
