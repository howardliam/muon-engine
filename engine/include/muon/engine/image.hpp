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
        [[nodiscard]] vk::Extent2D getExtent();

        /**
         * @brief   get image layout.
         *
         * @return  layout of the image.
         */
        [[nodiscard]] vk::ImageLayout getImageLayout();

        /**
         * @brief   get image format.
         *
         * @return  color format of the image.
         */
        [[nodiscard]] vk::Format getFormat();

        /**
         * @brief   get image handle.
         *
         * @return  image handle.
         */
        [[nodiscard]] vk::Image getImage();

        /**
         * @brief   get image view handle.
         *
         * @return  image view handle.
         */
        [[nodiscard]] vk::ImageView getImageView();

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
