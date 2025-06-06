#pragma once

#include <cstdint>
#include <string>

namespace muon::gfx {

    enum class SuitabilityFlagBits : uint64_t {
        ApiVersion13                = 0x8000000000000000,
        DiscreteGPU                 = 0x4000000000000000,
        MinimumPushConstantSize     = 0x2000000000000000,
        HasRequiredExtensions       = 0x1000000000000000,

        GraphicsQueue               = 0x0000000800000000,
        ComputeQueue                = 0x0000000400000000,
        TransferQueue               = 0x0000000200000000,
        UnifiedQueue                = 0x0000000100000000,
    };

    class GpuSuitability {
    public:
        GpuSuitability() = default;
        ~GpuSuitability() = default;

        bool operator==(const GpuSuitability &other) const;

        bool GetFlag(const SuitabilityFlagBits &flag) const;
        void SetFlag(const SuitabilityFlagBits &flag);

        std::string ToString() const;

    private:
        uint64_t m_suitability{0};
    };

}
