#pragma once

namespace muon::asset {

    enum class AssetLoadError {
        FileNotFound,
        InvalidFormat,
        ParseError,
    };

}
