#pragma once

#include "muon/core/layer.hpp"

#include <vector>

namespace muon {

class LayerStack {
public:
    using ValueType = std::vector<Layer *>;
    using SizeType = ValueType::size_type;
    using Iterator = ValueType::iterator;
    using ConstIterator = ValueType::const_iterator;
    using ReverseIterator = ValueType::reverse_iterator;
    using ConstReverseIterator = ValueType::const_reverse_iterator;

    LayerStack();
    ~LayerStack();

    void push_layer(Layer *layer);
    void pop_layer(Layer *layer);

    auto begin() noexcept -> Iterator;
    auto begin() const noexcept -> ConstIterator;

    auto end() noexcept -> Iterator;
    auto end() const noexcept -> ConstIterator;

    auto rbegin() noexcept -> ReverseIterator;
    auto rbegin() const noexcept -> ConstReverseIterator;

    auto rend() noexcept -> ReverseIterator;
    auto rend() const noexcept -> ConstReverseIterator;

    auto size() const noexcept -> SizeType;

private:
    ValueType layers_{};
    Iterator layer_insert_;
};

} // namespace muon
