#pragma once

namespace muon {

template <typename T>
concept TransientResource = T::transientResource;

}
