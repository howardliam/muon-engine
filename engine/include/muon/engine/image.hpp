#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <memory>

namespace muon::engine {

    class Device;

    class Image : NoCopy, NoMove {
    public:
        class Builder;

        struct State {
            vk::ImageLayout imageLayout{};
            vk::AccessFlags accessFlags{};
            vk::PipelineStageFlags pipelineStageFlags{};
        };

        Image(
            Device &device,
            vk::Extent2D extent,
            vk::Format format,
            vk::ImageUsageFlags usageFlags,
            const State &state
        );
        ~Image();

        /**
         * @brief   transitions the image to the desired layout, can only be used once before transition must be reverted.
         *
         * @param   commandBuffer   command buffer to record to.
         * @param   newState        the new state to transition to image to.
         */
        void transitionLayout(vk::CommandBuffer commandBuffer, const State &newState);

        /**
         * @brief   reverts the transition.
         *
         * @param   commandBuffer   command buffer to record to.
         */
        void revertTransition(vk::CommandBuffer commandBuffer);

        /**
         * @brief   get image extent.
         *
         * @return  size of the image.
         */
        [[nodiscard]] vk::Extent2D getExtent() const;

        /**
         * @brief   get image layout.
         *
         * @return  layout of the image.
         */
        [[nodiscard]] vk::ImageLayout getImageLayout() const;

        /**
         * @brief   get image format.
         *
         * @return  color format of the image.
         */
        [[nodiscard]] vk::Format getFormat() const;

        /**
         * @brief   get image handle.
         *
         * @return  image handle.
         */
        [[nodiscard]] vk::Image getImage() const;

        /**
         * @brief   get image view handle.
         *
         * @return  image view handle.
         */
        [[nodiscard]] vk::ImageView getImageView() const;

        /**
         * @brief   get image descriptor info.
         *
         * @return  descriptor information struct.
         */
        [[nodiscard]] vk::DescriptorImageInfo getDescriptorInfo() const;

    private:
        Device &device;

        vk::Extent2D extent;
        vk::Format format;
        vk::ImageUsageFlags usageFlags;

        State state;

        bool transitioned{false};
        State oldState{};

        vk::Image image;
        vma::Allocation allocation;
        vk::ImageView imageView;

        /**
         * @brief   creates the image and transitions it to the desired format.
         */
        void createImage();
    };

    class Image::Builder {
    public:
        Builder(Device &device);

        Builder &setExtent(vk::Extent2D extent);

        Builder &setFormat(vk::Format format);

        Builder &setImageUsageFlags(vk::ImageUsageFlags imageUsageFlags);

        Builder &setImageLayout(vk::ImageLayout imageLayout);

        Builder &setAccessFlags(vk::AccessFlags accessFlags);

        Builder &setPipelineStageFlags(vk::PipelineStageFlags pipelineStageFlags);

        Image build() const;
        std::unique_ptr<Image> buildUniquePtr() const;

    private:
        Device &device;

        vk::Extent2D extent;
        vk::Format format;
        vk::ImageUsageFlags imageUsageFlags;

        vk::ImageLayout imageLayout;
        vk::AccessFlags accessFlags;
        vk::PipelineStageFlags pipelineStageFlags;
    };

}
