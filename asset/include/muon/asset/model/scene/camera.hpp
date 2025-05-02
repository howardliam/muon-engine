#pragma once

#include <optional>
#include <variant>
#include <string>

namespace muon::asset {

    struct OrthographicCamera {
        float xmag{0.0};
        float ymag{0.0};
        float zfar{0.0};
        float znear{0.0};
    };

    struct PerspectiveCamera {
        std::optional<float> aspectRatio;
        float yfov{0.0};
        std::optional<float> zfar{0.0};
        float znear{0.0};
    };

    struct Camera {
        std::variant<OrthographicCamera, PerspectiveCamera> camera;
        std::optional<std::string> name;
    };

}
