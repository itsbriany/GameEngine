#pragma once
#include <vulkan.hpp>
#include "vulkan_gpu.h"
#include <iostream>

namespace Phyre {
namespace Graphics {
class VulkanDevice;
class VulkanWindow;
class VulkanMemoryManager;

class VulkanSwapchain {
public:
    struct SwapchainImage {
        vk::Image image;
        vk::ImageView image_view;
    };
    struct DepthImage {
        DepthImage(const vk::Image& image, const vk::ImageView& image_view, vk::Format format, const vk::DeviceMemory& device_memory) :
            image(image), image_view(image_view), format(format), device_memory(device_memory) { }
        vk::Image image;
        vk::ImageView image_view;
        vk::Format format;
        vk::DeviceMemory device_memory;
    };

    typedef std::vector<SwapchainImage> SwapchainImageVector;

    explicit VulkanSwapchain(const VulkanDevice& device, const VulkanWindow& window);

    const VulkanWindow& window() const { return window_; }
    const SwapchainImageVector& swapchain_images() const { return swapchain_images_; }
    vk::Format depth_format() const { return depth_image_.format; }
    vk::SampleCountFlagBits samples() const { return samples_; }
    uint32_t image_width() const { return image_width_; }
    uint32_t image_height() const { return image_height_; }
    const DepthImage& depth_image() const { return depth_image_; }
    const vk::SwapchainKHR& swapchain() const { return swapchain_; }

    ~VulkanSwapchain();

private:
    // --------------- Type definitions -----------------
    typedef std::vector<vk::PresentModeKHR> PresentModes;
    typedef std::vector<vk::ImageView> ImageViewVector;


    // --------------- Initializers ---------------------
    static const std::string kWho;
    
    // Returns the extent of the buffer we will render to
    // The width and height should be some default dimensions, but ideally, they should reflect those of an existing surface
    static vk::Extent2D InitializeSwapchainExtent(const VulkanWindow& window);
    
    // Returns the pre transform given the available surface capabilities
    static vk::SurfaceTransformFlagBitsKHR InitializePreTransform(const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    
    // Throws a runtime exception if the swapchain failed to initialize
    static vk::SwapchainKHR InitializeSwapchain(const VulkanDevice& device,
                                                const VulkanWindow& window,
                                                const vk::Extent2D& extent,
                                                const vk::SurfaceTransformFlagBitsKHR& pre_transform);

    // Throws a runtime exception if the swapchain images failed to instantiate
    static SwapchainImageVector InitializeSwapchainImages(const vk::Device& device, const vk::SwapchainKHR& swapchain, const vk::Format& format);

    // Throws a runtime exception if the depth buffer image failed to instantiate
    static DepthImage InitializeDepthImage(const VulkanDevice& device,
                                           uint32_t width,
                                           uint32_t height,
                                           vk::SampleCountFlagBits samples);

    // -------------------Data members -----------------
    // A reference to our window
    const VulkanWindow& window_;

    // A reference to the surface we want to send images to
    const vk::SurfaceKHR& surface_;
    
    // A reference to the window's width
    const uint32_t& image_width_;

    // A reference to the window's height
    const uint32_t& image_height_;

    // A reference to the vulkan device
    const VulkanDevice& device_;

    // The size of a rectangular retion of pixels within an image or framebuffer
    vk::Extent2D swapchain_extent_;

    // The transform relative to the presentation engine's natural orientation
    vk::SurfaceTransformFlagBitsKHR pre_transform_;

    // The underlying vulkan swapchain
    vk::SwapchainKHR swapchain_;

    // The images we swap
    SwapchainImageVector swapchain_images_;

    // How many image samples we are using
    vk::SampleCountFlagBits samples_;

    // The depth image
    DepthImage depth_image_;
};

}
}
