#pragma once

#include "muon/core/buffer.hpp"
#include "muon/serde/serde.hpp"

namespace muon::serde {

template <typename From>
concept BinarySerializable = Serializable<From, Buffer>;

enum class BinaryDeserializeError {};

template <typename To>
concept Binarydeserializable = Deserializable<BufferView, To, BinaryDeserializeError>;

}
