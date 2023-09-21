#include "zvar.h"

#include "../dck.h"

#include <stdio.h>
#include <stdbool.h>
// TODO: Remove.
#include <assert.h>

#define lengthof(arr) (sizeof(arr) / sizeof(*arr))

#ifdef _DEBUG
    #define unreachable()                                                           \
    do {                                                                            \
        fprintf(stderr, "%s:%d: Unreachable line reached.\n", __FILE__, __LINE__);  \
        exit(666);                                                                  \
    } while (0)
#else
    #ifdef _WIN32
        #define unreachable()   __assume(0)
    #else
        #define unreachable()   __builtin_unreachable()
    #endif
#endif


static char *default_validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
};

static char *default_instance_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
};

static char *default_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

// NOTE: Ordered from most prefered to least.
static VkFormat default_surface_formats[] = {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_R16G16B16A16_SFLOAT,
};

// NOTE: Ordered from most prefered to least.
static VkColorSpaceKHR default_surface_color_spaces[] = {
    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
};

// NOTE: Ordered from most prefered to least.
//       FIFO is guaranteed to exist.
static VkPresentModeKHR default_vsync_present_mode_prefs[] = {
    VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    VK_PRESENT_MODE_FIFO_KHR,
};

// NOTE: Ordered from most prefered to least.
//       FIFO is guaranteed to exist.
static VkPresentModeKHR default_nosync_present_mode_prefs[] = {
    VK_PRESENT_MODE_MAILBOX_KHR,
    VK_PRESENT_MODE_IMMEDIATE_KHR,
    VK_PRESENT_MODE_FIFO_KHR,
};

// NOTE: Ordered from most prefered to least.
//       One of them is guaranteed to be supported.
static VkCompositeAlphaFlagBitsKHR default_composite_alpha_prefs[] = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

// NOTE: Ordered from most prefered to least.
static const VkFormat default_depth_format_prefs[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
};


void zvar_vulkan_handle_error(VkResult res, const char *file, const char *function, int line)
{
    (void)res;

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "zvar error in `%s`, %s:%d\n", function, file, line);
    zvar_error(buffer);
}


static bool str_eq(char *str_a, char *str_b)
{
    for (; *str_a && *str_b; ++str_a, ++str_b) {
        if (*str_a != *str_b)
            return false;
    }

    return *str_a == *str_b;
}


static dck_stretchy_t (uint8_t, uint32_t) scratch;

static void *zvar_get_scratch(uint32_t size)
{
    scratch.count = 0;
    dck_stretchy_reserve(scratch, size);

    return scratch.data;
}


VkCommandPool zvar_create_command_pool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queue_family_index,
    };

    VkCommandPool res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateCommandPool(device, &create_info, NULL, &res));

    return res;
}


void zvar_allocate_command_buffers(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel secondary, uint32_t count, VkCommandBuffer *command_buffers)
{
    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = secondary,
        .commandBufferCount = count,
    };

    ZVAR_CHECK(vkAllocateCommandBuffers(device, &allocate_info, command_buffers));
}


VkSemaphore zvar_create_semaphore(VkDevice device)
{
    VkSemaphoreCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkSemaphore res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateSemaphore(device, &create_info, NULL, &res));

    return res;
}


VkFence zvar_create_fence(VkDevice device, VkFenceCreateFlags signaled)
{
    VkFenceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = signaled,
    };

    VkFence res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateFence(device, &create_info, NULL, &res));

    return res;
}


VkShaderModule zvar_create_shader_module(VkDevice device, size_t size, void *code)
{
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (uint32_t *)code,
    };

    VkShaderModule res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateShaderModule(device, &create_info, NULL, &res));

    return res;
}


VkBuffer zvar_create_buffer_exclusive(VkDevice device, VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .flags = flags,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkBuffer res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateBuffer(device, &create_info, NULL, &res));

    return res;
}


VkMemoryRequirements zvar_get_buffer_memory_requirements(VkDevice device, VkBuffer buffer)
{
    VkMemoryRequirements res;

    vkGetBufferMemoryRequirements(device, buffer, &res);

    return res;
}


VkMemoryRequirements zvar_get_image_memory_requirements(VkDevice device, VkImage image)
{
    VkMemoryRequirements res;

    vkGetImageMemoryRequirements(device, image, &res);

    return res;
}


VkCommandBuffer zvar_begin_one_off_command_buffer(VkDevice device, VkCommandPool command_pool)
{
    VkCommandBuffer res = VK_NULL_HANDLE;

    zvar_allocate_command_buffers(device, command_pool, false, 1, &res);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    ZVAR_CHECK(vkBeginCommandBuffer(res, &begin_info));

    return res;
}


void zvar_finish_one_off_command_buffer(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer)
{
    ZVAR_CHECK(vkEndCommandBuffer(command_buffer));

    VkFence fence = zvar_create_fence(device, false);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    ZVAR_CHECK(vkQueueSubmit(queue, 1, &submit_info, fence));

    ZVAR_CHECK(vkWaitForFences(device, 1, &fence, false, ~0ull));

    vkDestroyFence(device, fence, NULL);
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}


VkImage zvar_create_2d_image_exclusive(VkDevice device, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels, VkImageUsageFlags usage)
{
    VkImage res = VK_NULL_HANDLE;

    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width  = width,
            .height = height,
            .depth = 1,
        },
        .mipLevels = mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    ZVAR_CHECK(vkCreateImage(device, &create_info, NULL, &res));

    return res;
}


VkDeviceMemory zvar_allocate_memory(VkDevice device, uint32_t memory_type, VkDeviceSize size)
{
    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = size,
        .memoryTypeIndex = memory_type,
    };

    VkDeviceMemory res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkAllocateMemory(device, &allocate_info, NULL, &res));

    return res;
}


VkImageView zvar_create_2d_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, uint32_t level_count)
{
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = { // NOTE: Optional.
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange = {
            .aspectMask = aspect_mask,
            .baseMipLevel = 0,
            .levelCount = level_count,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView res = VK_NULL_HANDLE;

    ZVAR_CHECK(vkCreateImageView(device, &create_info, NULL, &res));

    return res;
}

int32_t zvar_find_memory_type(VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t supported_type_mask, VkMemoryPropertyFlags required_properties)
{
    uint32_t type_count = memory_properties->memoryTypeCount;

    for (uint32_t index = 0; index < type_count; ++index) {
        if (!(supported_type_mask & (1 << index)))
            continue;

        VkMemoryPropertyFlags properties = memory_properties->memoryTypes[index].propertyFlags;

        if ((properties & required_properties) == required_properties)
            return (int32_t)index;
    }

    return -1;
}

// TODO: Make depth parameters nullable.
bool zvar_create_swapchain_clique(const zvar_swapchain_create_info_t *info,
                                  VkSwapchainKHR *swapchain, uint32_t *width, uint32_t *height, uint32_t *image_count, VkImageView *views, VkFramebuffer *framebuffers,
                                  VkImage *depth_image, VkDeviceMemory *depth_memory, VkImageView *depth_view)
{
    // create swapchain
    {
        VkSwapchainKHR old_swapchain = *swapchain;

        VkSurfaceCapabilitiesKHR surface_capabilities;
        ZVAR_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info->physical_device, info->surface, &surface_capabilities));

        uint32_t present_mode_count;
        ZVAR_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(info->physical_device, info->surface, &present_mode_count, NULL));
        VkPresentModeKHR *present_modes = zvar_get_scratch(present_mode_count * sizeof(VkPresentModeKHR));
        ZVAR_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(info->physical_device, info->surface, &present_mode_count, present_modes));

        uint32_t present_mode_pref_count = info->present_mode_pref_count;
        VkPresentModeKHR *present_mode_prefs = info->present_mode_prefs;

        if (present_mode_prefs == ZVAR_VSYNC_DEFAULT_PRESENT_MODE) {
            present_mode_pref_count = lengthof(default_vsync_present_mode_prefs);
            present_mode_prefs      = default_vsync_present_mode_prefs;
        }
        else if (present_mode_prefs == ZVAR_NOSYNC_DEFAULT_PRESENT_MODE) {
            present_mode_pref_count = lengthof(default_nosync_present_mode_prefs);
            present_mode_prefs      = default_nosync_present_mode_prefs;
        }

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

        for (uint32_t pi = 0; pi < present_mode_pref_count; ++pi) {
            VkPresentModeKHR p = present_mode_prefs[pi];

            for (uint32_t i = 0; i < present_mode_count; ++i) {
                if (present_modes[i] == p) {
                    present_mode = p;
                    break;
                }
            }

            if (present_mode == p)
                break;
        }

        VkExtent2D swapchain_extent = surface_capabilities.currentExtent;

        // In this case width == height.
        if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
            swapchain_extent = (VkExtent2D) {
                .width  = *width,
                .height = *height,
            };

            if (swapchain_extent.width < surface_capabilities.minImageExtent.width) {
                swapchain_extent.width = surface_capabilities.minImageExtent.width;
            }
            else if (swapchain_extent.width > surface_capabilities.maxImageExtent.width) {
                swapchain_extent.width = surface_capabilities.maxImageExtent.width;
            }

            if (swapchain_extent.height < surface_capabilities.minImageExtent.height) {
                swapchain_extent.height = surface_capabilities.minImageExtent.height;
            }
            else if (swapchain_extent.height > surface_capabilities.maxImageExtent.height) {
                swapchain_extent.height = surface_capabilities.maxImageExtent.height;
            }
        }
        else {
            *width  = swapchain_extent.width;
            *height = swapchain_extent.height;
        }

        if (surface_capabilities.currentExtent.width == 0 || surface_capabilities.currentExtent.height == 0) {
            return false;
        }

        uint32_t swapchain_image_count = info->prefered_image_count;

        if (swapchain_image_count < surface_capabilities.minImageCount) {
            swapchain_image_count = surface_capabilities.minImageCount;
        }

        if (surface_capabilities.maxImageCount && swapchain_image_count > surface_capabilities.maxImageCount) {
            swapchain_image_count = surface_capabilities.maxImageCount;
        }

        assert(swapchain_image_count <= info->maximum_image_count);

        *image_count = swapchain_image_count;

        VkSurfaceTransformFlagBitsKHR surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

        if (!(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)) {
            surface_transform = surface_capabilities.currentTransform;
        }

        uint32_t composite_alpha_pref_count = info->composite_alpha_pref_count;
        VkCompositeAlphaFlagBitsKHR *composite_alpha_prefs = info->composite_alpha_prefs;

        if (composite_alpha_prefs == NULL) {
            composite_alpha_pref_count = lengthof(default_composite_alpha_prefs);
            composite_alpha_prefs      = default_composite_alpha_prefs;
        }

        VkCompositeAlphaFlagBitsKHR composite_alpha = 0;
        for (uint32_t i = 0; i < composite_alpha_pref_count; ++i) {
            VkCompositeAlphaFlagBitsKHR ca = composite_alpha_prefs[i];

            if (surface_capabilities.supportedCompositeAlpha & ca) {
                composite_alpha = ca;
                break;
            }
        }

        VkSwapchainCreateInfoKHR swapchain_create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = info->surface,
            .minImageCount = swapchain_image_count,
            .imageFormat = info->surface_format.format,
            .imageColorSpace = info->surface_format.colorSpace,
            .imageExtent = swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surface_transform,
            .compositeAlpha = composite_alpha,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = old_swapchain,
        };

        ZVAR_CHECK(vkCreateSwapchainKHR(info->device, &swapchain_create_info, NULL, swapchain));

        if (old_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(info->device, old_swapchain, NULL);
        }
    }

    // create depth buffer
    {
        *depth_image = zvar_create_2d_image_exclusive(info->device, info->depth_format, *width, *height, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        VkMemoryRequirements depth_memory_requirements = zvar_get_image_memory_requirements(info->device, *depth_image);

        uint32_t depth_memory_type = zvar_find_memory_type(info->physical_device_memory_properties, depth_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // TODO: Reuse the memory when stretching. Also probably preallocate enough memory for the whole screen.
        *depth_memory = zvar_allocate_memory(info->device, depth_memory_type, depth_memory_requirements.size);

        ZVAR_CHECK(vkBindImageMemory(info->device, *depth_image, *depth_memory, 0));

        VkImageAspectFlags depth_aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT | (info->depth_format == VK_FORMAT_D32_SFLOAT ? 0 : VK_IMAGE_ASPECT_STENCIL_BIT);

        *depth_view = zvar_create_2d_image_view(info->device, *depth_image, info->depth_format, depth_aspect_flags, 1);
    }

    // retrieve swapchain images and create views
    {
        VkImage *images = zvar_get_scratch(*image_count * sizeof(VkImage));

        ZVAR_CHECK(vkGetSwapchainImagesKHR(info->device, *swapchain, image_count, images));

        for (uint32_t i = 0; i < *image_count; ++i) {
            views[i] = zvar_create_2d_image_view(info->device, images[i], info->surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    // create framebuffers
    {
        for (uint32_t i = 0; i < *image_count; ++i) {
            VkFramebufferCreateInfo framebuffer_create_info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = info->render_pass,
                .attachmentCount = 2,
                .pAttachments = (VkImageView[2]) {
                    [0] = views[i],
                    [1] = *depth_view,
                },
                .width  = *width,
                .height = *height,
                .layers = 1,
            };


            ZVAR_CHECK(vkCreateFramebuffer(info->device, &framebuffer_create_info, NULL, framebuffers + i));
        }
    }

    return true;
}

VkInstance zvar_create_instance(const zvar_instance_create_info_t *info)
{
    if (volkInitialize() != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    // get the version of api
    {
        uint32_t api_version = VK_API_VERSION_1_0;

        if (vkEnumerateInstanceVersion) {
            if (vkEnumerateInstanceVersion(&api_version) != VK_SUCCESS)
                return VK_NULL_HANDLE;
        }

        // TODO: Remove maybe? Or do differently.
        printf("Vulkan API version: %d.%d.%d:%d\n",
               VK_API_VERSION_MAJOR(api_version),
               VK_API_VERSION_MINOR(api_version),
               VK_API_VERSION_PATCH(api_version),
               VK_API_VERSION_VARIANT(api_version));

        if (api_version < info->minimum_version) {
            return VK_NULL_HANDLE;
        }
    }

    // find validation layers
    char **layers = info->required_validation_layers;
    uint32_t layer_count = info->required_validation_layer_count;

    {
        if (layers == NULL) {
            layers = default_validation_layers;
            layer_count = lengthof(default_validation_layers);
        }

        uint32_t instance_layer_count;
        if (vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        VkLayerProperties *layer_properties = zvar_get_scratch(instance_layer_count * sizeof(VkLayerProperties));
        if (vkEnumerateInstanceLayerProperties(&instance_layer_count, layer_properties) != VK_SUCCESS)
            return VK_NULL_HANDLE;

        for (uint32_t i = 0; i < layer_count; ++i) {
            char *name = layers[i];

            bool found = false;

            for (uint32_t j = 0; j < instance_layer_count; ++j) {
                VkLayerProperties *props = layer_properties + j;

                if (str_eq(name, props->layerName)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                fprintf(stderr, "Required validation layer '%s' not found!\n", name);
                return VK_NULL_HANDLE;
            }
        }
    }

    // find instance extensions
    char **req_instance_extensions = info->required_instance_extensions;
    uint32_t req_instance_extension_count = info->required_instance_extension_count;

    {
        if (req_instance_extensions == NULL) {
            req_instance_extensions = default_instance_extensions;
            req_instance_extension_count = lengthof(default_instance_extensions);
        }

        uint32_t instance_extension_count;
        if (vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        VkExtensionProperties *extension_properties = zvar_get_scratch(instance_extension_count * sizeof(VkExtensionProperties));
        if (vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, extension_properties) != VK_SUCCESS)
            return VK_NULL_HANDLE;

        for (uint32_t i = 0; i < req_instance_extension_count; ++i) {
            char *name = req_instance_extensions[i];

            bool found = false;

            for (uint32_t j = 0; j < instance_extension_count; ++j) {
                VkExtensionProperties *props = extension_properties + j;

                if (str_eq(name, props->extensionName)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                fprintf(stderr, "Required instance extension '%s' not found!\n", name);
                return VK_NULL_HANDLE;
            }
        }
    }

    // create instance

    VkInstance instance;
    {
        VkInstanceCreateInfo instance_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = 0,
            .pApplicationInfo = &(VkApplicationInfo) {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = info->application_name ? info->application_name : "zvarog",
                .applicationVersion = info->application_version,
                .pEngineName = info->engine_name ? info->engine_name : "zvar",
                .engineVersion = info->engine_version,
                .apiVersion = info->minimum_version,
            },
            .enabledLayerCount = layer_count,
            .ppEnabledLayerNames = layers,
            .enabledExtensionCount = req_instance_extension_count,
            .ppEnabledExtensionNames = req_instance_extensions,
        };

        if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS)
            return VK_NULL_HANDLE;
    }

    volkLoadInstance(instance);

    return instance;
}

VkPhysicalDevice zvar_choose_some_physical_device(VkInstance instance)
{
    uint32_t physical_device_count;
    ZVAR_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL));
    VkPhysicalDevice *physical_devices = zvar_get_scratch(physical_device_count * sizeof(VkPhysicalDevice));
    ZVAR_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices));

    uint32_t device_type_counts[VK_PHYSICAL_DEVICE_TYPE_CPU + 1] = {0};

    for (uint32_t i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physical_devices[i], &props);

        device_type_counts[props.deviceType]++;
    }

    VkPhysicalDeviceType type_to_find;

    if (device_type_counts[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
        type_to_find = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }
    else if (device_type_counts[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
        type_to_find = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    }
    else if (device_type_counts[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
        type_to_find = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    }
    else if (device_type_counts[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
        type_to_find = VK_PHYSICAL_DEVICE_TYPE_CPU;
    }
    else if (device_type_counts[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
        type_to_find = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    }
    else {
        fprintf(stderr, "Failed to find GPU!\n");
        exit(1);
    }

    uint32_t device_number = 0;

    for (; device_number < physical_device_count; ++device_number) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physical_devices[device_number], &props);

        if (props.deviceType == type_to_find)
            break;
    }

    return physical_devices[device_number];
}

VkDevice zvar_create_device(const zvar_device_create_info_t *info, uint32_t *graphics_index, uint32_t *compute_index, uint32_t *transfer_index)
{
    char **req_device_extensions = info->required_device_extensions;
    uint32_t req_device_extension_count = info->required_device_extension_count;

    if (info->required_device_extensions == NULL) {
        req_device_extensions = default_device_extensions;
        req_device_extension_count = lengthof(default_device_extensions);
    }

    // find device extensions
    {
        uint32_t device_extension_count;
        ZVAR_CHECK(vkEnumerateDeviceExtensionProperties(info->physical_device, NULL, &device_extension_count, NULL));
        VkExtensionProperties *extension_properties = zvar_get_scratch(device_extension_count * sizeof(VkExtensionProperties));
        ZVAR_CHECK(vkEnumerateDeviceExtensionProperties(info->physical_device, NULL, &device_extension_count, extension_properties));

        for (uint32_t i = 0; i < req_device_extension_count; ++i) {
            char *name = req_device_extensions[i];

            bool found = false;

            for (uint32_t j = 0; j < device_extension_count; ++j) {
                VkExtensionProperties *props = extension_properties + j;

                if (str_eq(name, props->extensionName)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                fprintf(stderr, "Required device extension '%s' not found!\n", name);
                exit(1);
            }
        }
    }

    uint32_t graphics_family_index = ZVAR_NO_INDEX;
    uint32_t compute_family_index  = ZVAR_NO_INDEX;
    uint32_t transfer_family_index = ZVAR_NO_INDEX;

    // find adequate queue family
    {
        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(info->physical_device, &queue_family_count, NULL);
        VkQueueFamilyProperties *queue_family_properties = zvar_get_scratch(queue_family_count * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(info->physical_device, &queue_family_count, queue_family_properties);

        for (uint32_t family_index = 0; family_index < queue_family_count; ++family_index) {
            VkQueueFamilyProperties *props = queue_family_properties + family_index;

            VkBool32 supports_present;

            ZVAR_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(info->physical_device, family_index, info->surface, &supports_present));

            if (props->queueFlags & VK_QUEUE_GRAPHICS_BIT && supports_present) {
                graphics_family_index = family_index;
                continue;
            }
            //                                              Needs to be retarded because of present support.
            if (props->queueFlags & VK_QUEUE_COMPUTE_BIT && !(props->queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                compute_family_index = family_index;
                continue;
            }
            //                                               Needs to be retarded because of present support.
            if (props->queueFlags & VK_QUEUE_TRANSFER_BIT && !(props->queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(props->queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                transfer_family_index = family_index;
                continue;
            }
        }
    }

    VkDevice device;

    // create device
    {
        uint32_t device_queue_create_info_count = 0;
        VkDeviceQueueCreateInfo device_queue_create_infos[3];

        if (graphics_index) {
            if (graphics_family_index == ZVAR_NO_INDEX) {
                fprintf(stderr, "Failed to find graphics queue with a present capability!\n");
                exit(1);
            }

            *graphics_index = graphics_family_index;

            device_queue_create_infos[device_queue_create_info_count++] = (VkDeviceQueueCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = graphics_family_index,
                .queueCount = 1,
                .pQueuePriorities = &(float){ 1.0f },
            };
        }

        if (compute_index) {
            *compute_index = compute_family_index;

            if (compute_family_index != ZVAR_NO_INDEX) {
                device_queue_create_infos[device_queue_create_info_count++] = (VkDeviceQueueCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = compute_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = &(float){ 1.0f },
                };
            }
        }

        if (transfer_index) {
            *transfer_index = transfer_family_index;

            if (transfer_family_index != ZVAR_NO_INDEX) {
                device_queue_create_infos[device_queue_create_info_count++] = (VkDeviceQueueCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = transfer_family_index,
                    .queueCount = 1,
                    .pQueuePriorities = &(float){ 1.0f },
                };
            }
        }

        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = device_queue_create_info_count,
            .pQueueCreateInfos = device_queue_create_infos,
            .enabledExtensionCount = req_device_extension_count,
            .ppEnabledExtensionNames = req_device_extensions,
            .pEnabledFeatures = &info->requested_device_features,
        };

        ZVAR_CHECK(vkCreateDevice(info->physical_device, &device_create_info, NULL, &device));
    }

    return device;
}

VkSurfaceFormatKHR zvar_find_surface_format(const zvar_surface_format_info_t *info)
{
    VkFormat *supported_formats = info->supported_surface_formats;
    uint32_t supported_format_count = info->supported_surface_format_count;

    if (supported_formats == NULL) {
        supported_formats = default_surface_formats;
        supported_format_count = lengthof(default_surface_formats);
    }

    VkColorSpaceKHR *supported_color_spaces = info->supported_surface_color_spaces;
    uint32_t supported_color_space_count = info->supported_surface_color_space_count;

    if (supported_color_spaces == NULL) {
        supported_color_spaces = default_surface_color_spaces;
        supported_color_space_count = lengthof(default_surface_color_spaces);
    }

    uint32_t surface_format_count;
    ZVAR_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(info->physical_device, info->surface, &surface_format_count, NULL));
    VkSurfaceFormatKHR *surface_formats = zvar_get_scratch(surface_format_count * sizeof(VkSurfaceFormatKHR));
    ZVAR_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(info->physical_device, info->surface, &surface_format_count, surface_formats));

    uint32_t supported_format_index = supported_format_count;
    uint32_t supported_space_index  = supported_color_space_count;
    uint32_t found_format_index = 0;

    for (uint32_t i = 0; i < surface_format_count; ++i) {
        VkFormat format = surface_formats[i].format;

        for (uint32_t format_index = 0;
             format_index <= supported_format_index && format_index < supported_format_count;
             ++format_index)
        {
            if (format == supported_formats[format_index]) {
                VkColorSpaceKHR space = surface_formats[i].colorSpace;

                uint32_t search_count = (format_index == supported_format_index)
                                      ? supported_space_index
                                      : supported_color_space_count;

                uint32_t space_index = 0;

                for (; space_index < search_count; ++space_index) {
                    if (space == supported_color_spaces[space_index])
                        break;
                }

                if (space_index == search_count)
                    continue;

                supported_format_index = format_index;
                supported_space_index = space_index;
                found_format_index = i;
                break;
            }
        }
    }

    if (supported_format_index == supported_format_count) {
        fprintf(stderr, "Failed to find supported surface format!\n");
        exit(1);
    }

    return surface_formats[found_format_index];
}


VkFormat zvar_find_depth_format(const zvar_depth_format_info_t *info)
{
    const VkFormat *depth_format_prefs = info->supported_depth_formats;
    uint32_t depth_format_pref_count = info->supported_depth_format_count;

    if (depth_format_prefs == NULL) {
        depth_format_prefs = default_depth_format_prefs;
        depth_format_pref_count = lengthof(default_depth_format_prefs);
    }

    VkFormat res = 0;

    uint32_t i = 0;

    for (; i < depth_format_pref_count; ++i) {
        VkFormat format = depth_format_prefs[i];

        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(info->physical_device, format, &format_properties);

        if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            res = format;
            break;
        }
    }

    if (i == depth_format_pref_count) {
        fprintf(stderr, "Failed to find suitable depth format!\n");
        exit(EXIT_FAILURE);
    }

    return res;
}
