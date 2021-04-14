/*************************************************************************/
/*  vulkan_context_android.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "vulkan_context_android.h"
#include "platform/android/vulkan/android_pre_rotation.h"

#include <vulkan/vulkan_android.h>

const char *VulkanContextAndroid::_get_platform_surface_extension() const {
	return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
}

int VulkanContextAndroid::window_create(ANativeWindow *p_window, DisplayServer::VSyncMode p_vsync_mode, int p_width, int p_height) {
	VkAndroidSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.window = p_window;

	VkSurfaceKHR surface;
	VkResult err = vkCreateAndroidSurfaceKHR(_get_instance(), &createInfo, nullptr, &surface);
	if (err != VK_SUCCESS) {
		ERR_FAIL_V_MSG(-1, "vkCreateAndroidSurfaceKHR failed with error " + itos(err));
	}

	return _window_create(DisplayServer::MAIN_WINDOW_ID, p_vsync_mode, surface, p_width, p_height);
}

bool VulkanContextAndroid::_use_validation_layers() {
	uint32_t count = 0;
	_get_preferred_validation_layers(&count, nullptr);

	// On Android, we use validation layers automatically if they were explicitly linked with the app.
	return count > 0;
}

void VulkanContextAndroid::_choose_window_and_swap_chain_extent_(
		const VkExtent2D &currentWindowExtent, const VkSurfaceCapabilitiesKHR &surfCapabilities, VkExtent2D *windowExtent, VkExtent2D *swapChainExtent) {
	VulkanContext::_choose_window_and_swap_chain_extent_(currentWindowExtent, surfCapabilities, windowExtent, swapChainExtent);

	if (surfCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
			surfCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
		// Swap to get identity width and height
		std::swap(swapChainExtent->width, swapChainExtent->height);
		std::swap(windowExtent->width, windowExtent->height);
	}

	AndroidPreRotation::get_instance().set_current_transform(surfCapabilities.currentTransform);
}

VkSurfaceTransformFlagBitsKHR VulkanContextAndroid::_choose_pre_transform_(const VkSurfaceCapabilitiesKHR &surfCapabilities) {
	return surfCapabilities.currentTransform;
}
