#pragma once

#include "muon/core/buffer.hpp"
#include "muon/serde/serde.hpp"

namespace muon::serde {

template <typename Data>
concept BinarySerializable = Serializable<Data, Buffer>;

enum class BinaryDeserializeError {};

template <typename Data>
concept BinaryDeserializable = Deserializable<Data, Buffer, BinaryDeserializeError>;

}
