#pragma once

namespace muon::graphics {

template <typename T>
concept InFlightResource = T::inFlightResource;

}
