#pragma once

#include "muon/core/buffer.hpp"
#include "muon/serde/serde.hpp"

namespace muon::serde {

template <typename From>
concept binary_serializable = serializable<From, buffer>;

enum class binary_deserialize_error {};

template <typename To>
concept binary_deserializable = deserializable<buffer, To, binary_deserialize_error>;

}
