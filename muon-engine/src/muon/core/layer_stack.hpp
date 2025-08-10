#pragma once

#include "muon/core/layer.hpp"

#include <vector>

namespace muon {

class layer_stack {
public:
    using data_type = std::vector<layer *>;
    using size_type = data_type::size_type;
    using iterator = data_type::iterator;
    using const_iterator = data_type::const_iterator;
    using reverse_iterator = data_type::reverse_iterator;
    using const_reverse_iterator = data_type::const_reverse_iterator;

    layer_stack();
    ~layer_stack();

    void push_layer(layer *layer);
    void pop_layer(layer *layer);

    auto begin() noexcept -> iterator;
    auto begin() const noexcept -> const_iterator;

    auto end() noexcept -> iterator;
    auto end() const noexcept -> const_iterator;

    auto rbegin() noexcept -> reverse_iterator;
    auto rbegin() const noexcept -> const_reverse_iterator;

    auto rend() noexcept -> reverse_iterator;
    auto rend() const noexcept -> const_reverse_iterator;

    constexpr auto size() const noexcept -> size_type;

private:
    data_type layers_{};
    iterator layer_insert_;
};

} // namespace muon
