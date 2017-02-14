#pragma once
#include <vulkan.hpp>
#include "rendering_system_interface.h"
#include "loggable_interface.h"

namespace Phyre {
namespace Graphics {

class VulkanRenderingSystem : public RenderingSystemInterface, public Logging::LoggableInterface {
public:
    // Type definitions
    typedef std::shared_ptr<vk::PhysicalDevice> PtrPhysicalDevice;
    typedef std::vector<vk::PhysicalDevice> PhysicalDeviceVector;
    typedef std::vector<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfoVector;
    typedef std::vector<vk::CommandBuffer> CommandBufferVector;

    // Construction and destruction
    VulkanRenderingSystem();

    // Destroys all the allocated vulkan objects
    virtual ~VulkanRenderingSystem();

    // Returns true if the vulkan instance was instantiated
    bool InitializeVulkanInstance();

    // Returns true if physical devices could be initialized
    bool InitializePhysicalDevices();

    // Returns a vk::DeviceQueueCreateInfo associated with its physical device
    // Also sets the active device
    vk::DeviceQueueCreateInfo InitializeSupportedQueueIndices();

    // Returns true if a logical device could be created from the physical device
    bool InitializeLogicalDevice();

    // Returns true if a command buffer could be created from the logical device
    bool InitializeCommandBuffers();

    // Returns true if a surface was instantiated
    bool InitializeSurface();

    // Returns true if a swap chain was instantiated
    bool InitializeSwapchain();

    // Loggable interface overrides
    std::string log() override {
        return "[RenderingSystem]";
    }

    // Get a reference to the Vulkan instance
    const vk::Instance& instance() const { return vk_instance_; }


private:
    // Returns true if we have validation layer support
    static bool CheckValidationLayerSupport();

    // Returns true if GLFW was initialized
    static bool InitializeGLFW();

    // According to the Vulkan 1.0 spec, we need to enable the following extensions:
    // WSI (Window System Integration): For presenting to surfaces
    static std::vector<const char*> InstanceExtensionNames();
    
    // We can add the swapchain extension here since which is a device extension
    static std::vector<const char*> DeviceExtensionNames();

    // Specifies all of the validiation layers we wish to use.
    // All validation layers are used by default, but you can check
    // https://vulkan.lunarg.com/doc/sdk/1.0.39.1/windows/validation_layer_details.html
    // for more information on this
    static std::vector<const char*> InstanceLayerNames();

    // Our context
    vk::Instance vk_instance_;

    // The Vulkan Physical devices
    PhysicalDeviceVector physical_devices_;

    // Points to the Vulkan Physical device we are currently using
    const vk::PhysicalDevice* p_active_physical_device_ = nullptr;

    // The queue family index representing the queue family on the physical device
    // that has graphics capability
    uint32_t graphics_queue_family_index_;

    // The queue family index representing the queue family on the physical device
    // tha sends images to the surface
    uint32_t presentation_queue_family_index_;

    // The Vulkan logical device derived from one of the
    // VkInstance's physical devices
    vk::Device device_;

    // The command buffer to which we send Vulkan commands to
    CommandBufferVector command_buffers_;

    // The surface we are using to render images
    // TODO: This eventually needs to be destroyed
    vk::SurfaceKHR surface_;


    #ifdef NDEBUG
        const static bool kEnableValidationLayers = false;
    #else
        const static bool kEnableValidationLayers = true;
    #endif
};

}
}
