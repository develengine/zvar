/* TODO:
 * [ ] Add optional layers and extensions.
 * [ ] Remove dck.h from implementation.
 * [ ] Make prefered device work properly.
 * [X] Error callback.
 * [ ] Custom allocators.
 */

#ifndef ZVAR_H_
#define ZVAR_H_

/* Relies on volk:
 * https://github.com/zeux/volk
 */
#include "volk/volk.h"

#include <stdint.h>
#include <stdbool.h>


/* user defined */

/* Gets called when an error occures in zvar functions.
 * Only gets called for errors that are not easily recoverable.
 */
void zvar_error(char *message);


/* library */

void zvar_vulkan_handle_error(VkResult res, const char *file, const char *function, int line);

#define ZVAR_CHECK(...) \
do { \
    VkResult _m_res = __VA_ARGS__; \
    if (_m_res != VK_SUCCESS) { \
        zvar_vulkan_handle_error(_m_res, __FILE__, __FUNCTION__, __LINE__); \
    } \
} while (0)


#define ZVAR_EMPTY ((void *)666)

typedef struct
{
    uint32_t minimum_version;

    char *application_name;
    uint32_t application_version;
    char *engine_name;
    uint32_t engine_version;

    uint32_t required_validation_layer_count;
    char   **required_validation_layers;

    uint32_t required_instance_extension_count;
    char   **required_instance_extensions;
} zvar_instance_create_info_t;

VkInstance zvar_create_instance(const zvar_instance_create_info_t *info);


#define ZVAR_NO_INDEX 0xFFFFFFFF

typedef struct
{
    VkPhysicalDevice physical_device;
    VkSurfaceKHR surface;

    VkPhysicalDeviceFeatures requested_device_features;

    uint32_t required_device_extension_count;
    char   **required_device_extensions;
} zvar_device_create_info_t;

VkDevice zvar_create_device(const zvar_device_create_info_t *info, uint32_t *graphics_index, uint32_t *compute_index, uint32_t *transfer_index);


typedef struct
{
    VkPhysicalDevice physical_device;
    VkSurfaceKHR surface;

    /* In descending order of preference. */
    uint32_t  supported_surface_format_count;
    VkFormat *supported_surface_formats;

    /* In descending order of preference. */
    uint32_t         supported_surface_color_space_count;
    VkColorSpaceKHR *supported_surface_color_spaces;
} zvar_surface_format_info_t;

VkSurfaceFormatKHR zvar_find_surface_format(const zvar_surface_format_info_t *info);


typedef struct
{
    VkPhysicalDevice physical_device;

    uint32_t  supported_depth_format_count;
    VkFormat *supported_depth_formats;
} zvar_depth_format_info_t;

VkFormat zvar_find_depth_format(const zvar_depth_format_info_t *info);


#define ZVAR_VSYNC_DEFAULT_PRESENT_MODE  ((VkPresentModeKHR *)0)
#define ZVAR_NOSYNC_DEFAULT_PRESENT_MODE ((VkPresentModeKHR *)1)

typedef struct
{
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    VkPhysicalDeviceMemoryProperties *physical_device_memory_properties;
    VkFormat depth_format;
    VkRenderPass render_pass;

    uint32_t prefered_image_count;
    uint32_t maximum_image_count;

    uint32_t composite_alpha_pref_count;
    VkCompositeAlphaFlagBitsKHR *composite_alpha_prefs;

    uint32_t present_mode_pref_count;
    VkPresentModeKHR *present_mode_prefs;
} zvar_swapchain_create_info_t;

bool zvar_create_swapchain_clique(const zvar_swapchain_create_info_t *info,
                                  VkSwapchainKHR *swapchain, uint32_t *width, uint32_t *height, uint32_t *image_count, VkImageView *views, VkFramebuffer *framebuffers,
                                  VkImage *depth_image, VkDeviceMemory *depth_memory, VkImageView *depth_view);


VkPhysicalDevice zvar_choose_some_physical_device(VkInstance instance);

VkCommandPool zvar_create_command_pool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queue_family_index);

void zvar_allocate_command_buffers(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer *command_buffers);

VkSemaphore zvar_create_semaphore(VkDevice device);

VkFence zvar_create_fence(VkDevice device, VkFenceCreateFlags signaled);

VkShaderModule zvar_create_shader_module(VkDevice device, size_t size, void *code);

VkBuffer zvar_create_buffer_exclusive(VkDevice device, VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage);

VkMemoryRequirements zvar_get_buffer_memory_requirements(VkDevice device, VkBuffer buffer);

VkMemoryRequirements zvar_get_image_memory_requirements(VkDevice device, VkImage image);

VkCommandBuffer zvar_begin_one_off_command_buffer(VkDevice device, VkCommandPool command_pool);

void zvar_finish_one_off_command_buffer(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer);

VkImage zvar_create_2d_image_exclusive(VkDevice device, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels, VkImageUsageFlags usage);

VkDeviceMemory zvar_allocate_memory(VkDevice device, uint32_t memory_type, VkDeviceSize size);

VkImageView zvar_create_2d_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, uint32_t level_count);

int32_t zvar_find_memory_type(VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t supported_type_mask, VkMemoryPropertyFlags required_properties);

#endif // ZVAR_H_
