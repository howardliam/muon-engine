#pragma once

#include "muon/graphics/context.hpp"
#include "muon/graphics/descriptor_pool.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <expected>
#include <unordered_map>

namespace muon::graphics {

class DescriptorSetLayout {
public:
    struct Spec {
        const Context *context{nullptr};
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };

public:
    DescriptorSetLayout(const Spec &spec);
    ~DescriptorSetLayout();

    auto createDescriptorSet(const DescriptorPool &pool) -> std::expected<vk::raii::DescriptorSet, vk::Result>;

public:
    auto get() -> vk::raii::DescriptorSetLayout &;
    auto get() const -> const vk::raii::DescriptorSetLayout &;

    auto getBindings() -> std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &;

private:
    const Context &m_context;

    vk::raii::DescriptorSetLayout m_layout{nullptr};
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;
};

} // namespace muon::graphics
