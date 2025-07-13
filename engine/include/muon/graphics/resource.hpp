#pragma once

namespace muon::graphics {

template <typename T>
concept TransientResource = T::transientResource;

}
