#include "muon/schematic/shader.hpp"

#include <optional>

namespace muon::schematic {

    bool Shader::IsValid() const {
        bool pathPresent = path.has_value();
        bool binaryPresent = byteOffset.has_value() && byteLength.has_value();

        if (pathPresent && !binaryPresent) {
            return true;
        } else if (binaryPresent && !pathPresent) {
            return true;
        }

        return false;
    }

}
