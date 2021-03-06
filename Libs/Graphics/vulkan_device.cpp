#include <Logging/logging.h>
#include "vulkan_device.h"
#include "vulkan_instance.h"

const std::string Phyre::Graphics::VulkanDevice::kWho = "[VulkanDevice]";

Phyre::Graphics::VulkanDevice::VulkanDevice(const VulkanGPU& gpu, const VulkanWindow& window) :
    gpu_(gpu),
    graphics_queue_family_index_(InitializeGraphicsQueueIndex(gpu_.get())),
    presentation_queue_family_index_(InitializePresentationQueueIndex(gpu_.get(), window.surface(), graphics_queue_family_index_)),
    device_(InitializeLogicalDevice(gpu_.get())),
    graphics_queue_(InitializeGraphicsQueue(device_, graphics_queue_family_index_)),
    presentation_queue_(InitializePresentationQueue(device_, graphics_queue_, graphics_queue_family_index_, presentation_queue_family_index_))
{
    PHYRE_LOG(trace, kWho) << "Instantiated";
}

Phyre::Graphics::VulkanDevice::~VulkanDevice() {
    device_.destroy();
    PHYRE_LOG(trace, kWho) << "Destroyed";
}

vk::Device Phyre::Graphics::VulkanDevice::InitializeLogicalDevice(const vk::PhysicalDevice& gpu) {
    std::vector<float> queue_priorities;
    uint32_t max_queue_count = 1; // Let's only use one queue for now
    std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos(1, PrepareGraphicsQueueInfo(gpu, queue_priorities, max_queue_count));
    
    vk::DeviceCreateInfo device_create_info;
    std::vector<const char*> device_extension_names = DeviceExtentionNames();
    std::vector<const char*> device_layer_names;
    device_create_info.setPpEnabledExtensionNames(device_extension_names.data());
    device_create_info.setEnabledExtensionCount(static_cast<uint32_t>(device_extension_names.size()));
    device_create_info.setPpEnabledLayerNames(device_layer_names.data());
    device_create_info.setEnabledLayerCount(static_cast<uint32_t>(device_layer_names.size()));
    device_create_info.setQueueCreateInfoCount(static_cast<uint32_t>(device_queue_create_infos.size()));
    device_create_info.setPQueueCreateInfos(device_queue_create_infos.data());

#ifndef NDEBUG
    device_layer_names.emplace_back(VulkanInstance::kLunarGStandardValidation);
    device_create_info.setPpEnabledLayerNames(device_layer_names.data());
    device_create_info.setEnabledLayerCount(static_cast<uint32_t>(device_layer_names.size()));
#endif

    return gpu.createDevice(device_create_info);
}

vk::DeviceQueueCreateInfo Phyre::Graphics::VulkanDevice::PrepareGraphicsQueueInfo(const vk::PhysicalDevice& gpu, std::vector<float>& queue_priorities, uint32_t max_queue_count) {
    /**
    * Queues are categorized into families. We can think of families as GPU capabilities
    * such as Graphics, Compute, performing pixel block copies (blits), etc...
    * We can query the physical device to get a list of queues
    * that only represent the queue families we are interested in.
    */
    vk::DeviceQueueCreateInfo device_queue_create_info;
    std::vector<vk::QueueFamilyProperties> queue_family_properties_vector = gpu.getQueueFamilyProperties();
    for (const auto& queue_family_properties : queue_family_properties_vector) {
        for (uint32_t queue_index = 0; queue_index < queue_family_properties.queueCount; ++queue_index) {
            if (queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics) {
                // Index the queues to point to the graphics family and balance the load between them
                // by assigning them all the same queue priority level.
                device_queue_create_info.setQueueFamilyIndex(queue_index);
                
                // More queues means that we will need to use more memory
                if (max_queue_count >= queue_family_properties.queueCount) {
                     device_queue_create_info.setQueueCount(queue_family_properties.queueCount);
                     queue_priorities = std::vector<float>(queue_family_properties.queueCount, 1.0);
                } else {
                    device_queue_create_info.setQueueCount(max_queue_count);
                    queue_priorities = std::vector<float>(max_queue_count, 1.0);
                }
                 
                device_queue_create_info.setPQueuePriorities(queue_priorities.data());
                return device_queue_create_info;
            }
        }
    }
    PHYRE_LOG(warning, kWho) << "Could not find a queue family which supports graphics";
    return device_queue_create_info;
}

std::vector<const char*> Phyre::Graphics::VulkanDevice::DeviceExtentionNames() {
    return std::vector<const char*> {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

uint32_t Phyre::Graphics::VulkanDevice::InitializeGraphicsQueueIndex(const vk::PhysicalDevice& gpu) {
    std::vector<vk::QueueFamilyProperties> queue_family_properties_vector = gpu.getQueueFamilyProperties();
    for (const auto& queue_family_properties : queue_family_properties_vector) {
        for (uint32_t queue_index = 0; queue_index < queue_family_properties.queueCount; ++queue_index) {
            if (queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics) {
                return queue_index;
            }
        }
    }

    // Not found
    PHYRE_LOG(warning, kWho) << "Could not find a graphics queue index";
    return UINT32_MAX;
}

uint32_t Phyre::Graphics::VulkanDevice::InitializePresentationQueueIndex(const vk::PhysicalDevice& gpu, const vk::SurfaceKHR& surface, uint32_t graphics_queue_index) {
    uint32_t presentation_queue_index = UINT32_MAX;

    // Initialize a vector letting us know which queues currently support surface presentation
    std::vector<vk::Bool32> surface_support_vector(gpu.getQueueFamilyProperties().size());
    for (uint32_t queue_family_index = 0; queue_family_index < gpu.getQueueFamilyProperties().size(); ++queue_family_index) {
        gpu.getSurfaceSupportKHR(queue_family_index, surface, &surface_support_vector.data()[queue_family_index]);
    }

    // The provided gpu family index is also capable of presentation
    if (graphics_queue_index < surface_support_vector.size() && surface_support_vector[graphics_queue_index]) {
        return graphics_queue_index;
    }

    std::vector<vk::QueueFamilyProperties> queue_family_properties_vector = gpu.getQueueFamilyProperties();
    for (const auto& queue_family_properties : queue_family_properties_vector) {
        for (uint32_t queue_index = 0; queue_index < queue_family_properties.queueCount; ++queue_index) {
            if (queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics) {
                if (queue_index < surface_support_vector.size() && surface_support_vector[queue_index]) {
                    return queue_index;
                }
            }
        }
    }

    // If we did not find a graphics queue index, we still need to give a valid presentation queue index
    for (uint32_t queue_index = 0; queue_index < surface_support_vector.size(); ++queue_index) {
        if (surface_support_vector[queue_index]) {
            return queue_index;
        }
    }

    // Not found
    PHYRE_LOG(warning, kWho) << "Could not find a presentation queue index";
    return presentation_queue_index;
}

vk::Queue Phyre::Graphics::VulkanDevice::InitializeGraphicsQueue(vk::Device& device, uint32_t graphics_queue_family_index) {
    uint32_t queue_index = 0; // The first graphics queue index
    return device.getQueue(graphics_queue_family_index, queue_index);
}

vk::Queue Phyre::Graphics::VulkanDevice::InitializePresentationQueue(vk::Device& device,
                                                                      const vk::Queue& graphics_queue, 
                                                                      uint32_t graphics_queue_family_index,
                                                                      uint32_t presentation_queue_family_index) {
    if (graphics_queue_family_index == presentation_queue_family_index) {
        return graphics_queue;
    }
    uint32_t queue_index = 0;
    return device.getQueue(presentation_queue_family_index, queue_index);
}
