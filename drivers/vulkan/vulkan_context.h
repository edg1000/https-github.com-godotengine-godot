#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include "core/error_list.h"
#include "core/map.h"
#include "core/ustring.h"
#include <vulkan/vulkan.h>

class VulkanContext {

	enum {
		MAX_EXTENSIONS = 128,
		MAX_LAYERS = 64,
		FRAME_LAG = 2
	};

	bool use_validation_layers;

	VkInstance inst;
	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	VkPhysicalDeviceProperties gpu_props;
	uint32_t queue_family_count;
	VkQueueFamilyProperties *queue_props;
	VkDevice device;

	//present
	bool queues_initialized;
	uint32_t graphics_queue_family_index;
	uint32_t present_queue_family_index;
	bool separate_present_queue;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkColorSpaceKHR color_space;
	VkFormat format;
	VkSemaphore image_acquired_semaphores[FRAME_LAG];
	VkSemaphore draw_complete_semaphores[FRAME_LAG];
	VkSemaphore image_ownership_semaphores[FRAME_LAG];
	int frame_index;
	VkFence fences[FRAME_LAG];
	VkPhysicalDeviceMemoryProperties memory_properties;

	typedef struct {
		VkImage image;
		VkCommandBuffer graphics_to_present_cmd;
		VkImageView view;
		VkFramebuffer framebuffer;

	} SwapchainImageResources;

	struct Window {

		bool is_minimzed;
		VkSurfaceKHR surface;
		VkSwapchainKHR swapchain;
		SwapchainImageResources *swapchain_image_resources;
		VkPresentModeKHR presentMode;
		uint32_t current_buffer;
		int width;
		int height;
		VkCommandPool present_cmd_pool; //for separate present queue

		VkRenderPass render_pass;

		Window() {
			width = 0;
			height = 0;
			render_pass = VK_NULL_HANDLE;
			current_buffer = 0;
			surface = VK_NULL_HANDLE;
			swapchain_image_resources = VK_NULL_HANDLE;
			swapchain = VK_NULL_HANDLE;
			is_minimzed = false;
			presentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
	};

	Map<int, Window> windows;
	int last_window_id;
	uint32_t swapchainImageCount;

	//commands

	bool prepared;

	//extensions
	bool VK_KHR_incremental_present_enabled;
	bool VK_GOOGLE_display_timing_enabled;
	const char **instance_validation_layers;
	uint32_t enabled_extension_count;
	uint32_t enabled_layer_count;
	const char *extension_names[MAX_EXTENSIONS];
	const char *enabled_layers[MAX_LAYERS];

	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
	PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
	PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	PFN_vkGetRefreshCycleDurationGOOGLE fpGetRefreshCycleDurationGOOGLE;
	PFN_vkGetPastPresentationTimingGOOGLE fpGetPastPresentationTimingGOOGLE;

	VkDebugUtilsMessengerEXT dbg_messenger;

	Error _create_validation_layers();
	Error _initialize_extensions();

	VkBool32 _check_layers(uint32_t check_count, const char **check_names, uint32_t layer_count, VkLayerProperties *layers);
	static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData);

	Error _create_physical_device();

	Error _initialize_queues(VkSurfaceKHR surface);

	Error _create_device();

	Error _clean_up_swap_chain(Window *window);

	Error _update_swap_chain(Window *window);

	Error _create_swap_chain();
	Error _create_semaphores();

	Vector<VkCommandBuffer> command_buffer_queue;
	int command_buffer_count;

protected:
	virtual const char *_get_platform_surface_extension() const = 0;
	//	virtual VkResult _create_surface(VkSurfaceKHR *surface, VkInstance p_instance) = 0;

	virtual int _window_create(VkSurfaceKHR p_surface, int p_width, int p_height);

	VkInstance _get_instance() {
		return inst;
	}

	bool buffers_prepared;

public:
	VkDevice get_device();
	VkPhysicalDevice get_physical_device();
	int get_swapchain_image_count() const;
	uint32_t get_graphics_queue() const;

	void window_resize(int p_window_id, int p_width, int p_height);
	int window_get_width(int p_window = 0);
	int window_get_height(int p_window = 0);
	void window_destroy(int p_window_id);
	VkFramebuffer window_get_framebuffer(int p_window = 0);
	VkRenderPass window_get_render_pass(int p_window = 0);

	VkFormat get_screen_format() const;
	VkPhysicalDeviceLimits get_device_limits() const;

	void set_setup_buffer(const VkCommandBuffer &pCommandBuffer);
	void append_command_buffer(const VkCommandBuffer &pCommandBuffer);
	void resize_notify();
	void flush(bool p_flush_setup = false, bool p_flush_pending = false);
	Error prepare_buffers();
	Error swap_buffers();
	Error initialize();
	VulkanContext();
};

#endif // VULKAN_DEVICE_H
