#pragma once

#include "muon/engine/device.hpp"

namespace muon::engine {

    class Image {
    public:
        Image(
            Device &device,
            vk::Extent2D extent,
            vk::ImageLayout layout,
            vk::Format format,
            vk::ImageUsageFlags usageFlags
        );
        ~Image();

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
        vk::ImageLayout imageLayout;
        vk::Format format;
        vk::ImageUsageFlags usageFlags;

        vk::Image image;
        vma::Allocation allocation;
        vk::ImageView imageView;

        /**
         * @brief   creates the image.
         */
        void createImage();

        /**
         * @brief   transitions the image to the desired format.
         */
        void transitionImage();
    };

}
