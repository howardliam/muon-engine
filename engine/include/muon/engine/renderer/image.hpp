#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <memory>

namespace muon::engine {

    class Device;

    class Image : NoCopy, NoMove {
    public:
        class Builder;

        Image(
            Device &device,
            vk::Extent2D extent,
            vk::Format format,
            vk::ImageUsageFlags usageFlags,
            vk::ImageLayout imageLayout,
            vk::AccessFlags2 accessFlags,
            vk::PipelineStageFlags2 stageFlags
        );
        ~Image();

        /**
         * @brief   transitions the image to the desired layout, can only be used once before transition must be reverted.
         *
         * @param   cmd         command buffer to record to.
         * @param   newState    the new state to transition to image to.
         */
        void transitionLayout(
            vk::CommandBuffer cmd,
            vk::ImageLayout imageLayout,
            vk::AccessFlags2 accessFlags,
            vk::PipelineStageFlags2 stageFlags
        );

        void resize(vk::Extent2D extent);

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
        [[nodiscard]] vk::DescriptorImageInfo *getDescriptorInfo() const;

    private:
        Device &device;

        vk::Extent2D extent;
        vk::Format format;
        vk::ImageAspectFlags aspectFlags;
        vk::ImageUsageFlags usageFlags;

        vk::ImageLayout imageLayout{vk::ImageLayout::eUndefined};
        vk::AccessFlags2 accessFlags{};
        vk::PipelineStageFlags2 stageFlags{vk::PipelineStageFlagBits2::eTopOfPipe};

        vk::Image image;
        vma::Allocation allocation;
        vk::ImageView imageView;

        std::unique_ptr<vk::DescriptorImageInfo> descriptorInfo;

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

        Builder &setAccessFlags(vk::AccessFlags2 accessFlags);

        Builder &setPipelineStageFlags(vk::PipelineStageFlags2 stageFlags);

        Image build() const;
        std::unique_ptr<Image> buildUniquePtr() const;

    private:
        Device &device;

        vk::Extent2D extent;
        vk::Format format;
        vk::ImageUsageFlags imageUsageFlags;

        vk::ImageLayout imageLayout;
        vk::AccessFlags2 accessFlags;
        vk::PipelineStageFlags2 stageFlags;
    };

}
