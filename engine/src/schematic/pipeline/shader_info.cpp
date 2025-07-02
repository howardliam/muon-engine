#include "muon/schematic/pipeline/shader_info.hpp"

#include <optional>

namespace muon::schematic {

    auto ShaderInfo::IsValid() const -> bool {
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
