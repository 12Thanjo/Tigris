////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/Engine.h"

#include <Volk/volk.h>


namespace vulkan::utils{


	static auto print_handle_result_message(
		std::string_view function_name, std::string_view code_name, std::string_view description
	) -> void {
		#if defined(_DEBUG)
			evo::log::error("{}() | {} : {}", function_name, code_name, description);
		#else
			evo::log::error("{}() | {}", function_name, code_name);
		#endif
	}


	
	auto checkResult(VkResult result, std::string_view function_name) -> evo::Result<> {
		switch(result){
			case VK_SUCCESS: {
				return evo::Result<>();
			} break;

			case VK_NOT_READY: {
				print_handle_result_message(
					function_name,
					"VK_NOT_READY",
					"A fence or query has not yet completed"
				);
				return evo::resultError;
			} break;

			case VK_TIMEOUT: {
				print_handle_result_message(
					function_name,
					"VK_TIMEOUT",
					"A wait operation has not completed in the specified time"
				);
				return evo::resultError;
			} break;

			case VK_EVENT_SET: {
				print_handle_result_message(
					function_name,
					"VK_EVENT_SET",
					"An event is signaled"
				);
				return evo::resultError;
			} break;

			case VK_EVENT_RESET: {
				print_handle_result_message(
					function_name,
					"VK_EVENT_RESET",
					"An event is unsignaled"
				);
				return evo::resultError;
			} break;

			case VK_INCOMPLETE: {
				print_handle_result_message(
					function_name,
					"VK_INCOMPLETE",
					"A return array was too small for the result"
				);
				return evo::resultError;
			} break;

			case VK_ERROR_OUT_OF_HOST_MEMORY: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_OUT_OF_HOST_MEMORY",
					"A host memory allocation has failed."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_OUT_OF_DEVICE_MEMORY",
					"A device memory allocation has failed."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INITIALIZATION_FAILED: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INITIALIZATION_FAILED",
					"Initialization of an object could not be completed for implementation-specific reasons."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_DEVICE_LOST: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_DEVICE_LOST",
					"The logical or physical device has been lost. See Lost Device"
				);
				return evo::resultError;
			} break;

			case VK_ERROR_MEMORY_MAP_FAILED: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_MEMORY_MAP_FAILED",
					"Mapping of a memory object has failed."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_LAYER_NOT_PRESENT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_LAYER_NOT_PRESENT",
					"A requested layer is not present or could not be loaded."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_EXTENSION_NOT_PRESENT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_EXTENSION_NOT_PRESENT",
					"A requested extension is not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_FEATURE_NOT_PRESENT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_FEATURE_NOT_PRESENT",
					"A requested feature is not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INCOMPATIBLE_DRIVER: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INCOMPATIBLE_DRIVER",
					"The requested version of Vulkan is not supported by the driver "
						"or is otherwise incompatible for implementation-specific reasons."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_TOO_MANY_OBJECTS: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_TOO_MANY_OBJECTS",
					"Too many objects of the type have already been created."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_FORMAT_NOT_SUPPORTED: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_FORMAT_NOT_SUPPORTED",
					"A requested format is not supported on this device."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_FRAGMENTED_POOL: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_FRAGMENTED_POOL",
					"A pool allocation has failed due to fragmentation of the pool’s memory."
						"This must only be returned if no attempt to allocate host "
						"or device memory was made to accommodate the new allocation. "
						"This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, "
						"but only if the implementation is certain that the pool allocation failure was due to "
						"fragmentation."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_UNKNOWN: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_UNKNOWN",
					"An unknown error has occurred; either the application has provided invalid input, "
						"or an implementation failure has occurred."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_OUT_OF_POOL_MEMORY: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_OUT_OF_POOL_MEMORY",
					"A pool memory allocation has failed. This must only be returned if no attempt to allocate host "
						"or device memory was made to accommodate the new allocation. "
						"If the failure was definitely due to fragmentation of the pool, "
						"VK_ERROR_FRAGMENTED_POOL should be returned instead."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INVALID_EXTERNAL_HANDLE: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INVALID_EXTERNAL_HANDLE",
					"An external handle is not a valid handle of the specified type."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_FRAGMENTATION: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_FRAGMENTATION",
					"A descriptor pool creation has failed due to fragmentation."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS",
					"A buffer creation or memory allocation failed because the requested address is not available. "
						"A shader group handle assignment failed because "
						"the requested shader group handle information is no longer valid."
				);
				return evo::resultError;
			} break;

			case VK_PIPELINE_COMPILE_REQUIRED: {
				print_handle_result_message(
					function_name,
					"VK_PIPELINE_COMPILE_REQUIRED",
					"A requested pipeline creation would have required compilation, "
						"but the application requested compilation to not be performed."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_SURFACE_LOST_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_SURFACE_LOST_KHR",
					"A surface is no longer available."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR",
					"The requested window is already in use by Vulkan or another API "
						"in a manner which prevents it from being used again."
				);
				return evo::resultError;
			} break;

			case VK_SUBOPTIMAL_KHR: {
				print_handle_result_message(
					function_name,
					"VK_SUBOPTIMAL_KHR",
					"A swapchain no longer matches the surface properties exactly, "
						"but can still be used to present to the surface successfully."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_OUT_OF_DATE_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_OUT_OF_DATE_KHR",
					"A surface has changed in such a way that it is no longer compatible with the swapchain, "
						"and further presentation requests using the swapchain will fail. "
						"Applications must query the new surface properties and recreate their swapchain "
						"if they wish to continue presenting to the surface."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR",
					"The display used by a swapchain does not use the same presentable image layout, "
						"or is incompatible in a way that prevents sharing an image."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VALIDATION_FAILED_EXT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VALIDATION_FAILED_EXT",
					"A command failed because invalid usage was detected by the implementation or a validation-layer."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INVALID_SHADER_NV: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INVALID_SHADER_NV",
					"One or more shaders failed to compile or link. "
						"More details are reported back to the application via VK_EXT_debug_report if enabled."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR",
					"The requested VkImageUsageFlags are not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR",
					"The requested video picture layout is not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR",
					"A video profile operation specified via VkVideoProfileInfoKHR::videoCodecOperation "
						"is not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR",
					"Format parameters in a requested VkVideoProfileInfoKHR chain are not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR",
					"Codec-specific parameters in a requested VkVideoProfileInfoKHR chain are not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR",
					"The specified video Std header version is not supported."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT",
					"https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/"
						"VK_EXT_image_drm_format_modifier.html"
				);
				return evo::resultError;
			} break;

			case VK_ERROR_NOT_PERMITTED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_NOT_PERMITTED_KHR",
					"https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#VK_KHR_global_priority"
				);
				return evo::resultError;
			} break;

			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT",
					"An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT " 
						"failed as it did not have exclusive full-screen access. "
						"This may occur due to implementation-dependent reasons, outside of the application’s control."
				);
				return evo::resultError;
			} break;

			case VK_THREAD_IDLE_KHR: {
				print_handle_result_message(
					function_name,
					"VK_THREAD_IDLE_KHR",
					"A deferred operation is not complete but there is currently no work for this thread to do "
						"at the time of this call."
				);
				return evo::resultError;
			} break;

			case VK_THREAD_DONE_KHR: {
				print_handle_result_message(
					function_name,
					"VK_THREAD_DONE_KHR",
					"A deferred operation is not complete "
						"but there is no work remaining to assign to additional threads."
				);
				return evo::resultError;
			} break;

			case VK_OPERATION_DEFERRED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_OPERATION_DEFERRED_KHR",
					"A deferred operation was requested and at least some of the work was deferred."
				);
				return evo::resultError;
			} break;

			case VK_OPERATION_NOT_DEFERRED_KHR: {
				print_handle_result_message(
					function_name,
					"VK_OPERATION_NOT_DEFERRED_KHR",
					"A deferred operation was requested and no operations were deferred."
				);
				return evo::resultError;
			} break;

			#if defined(VK_ENABLE_BETA_EXTENSIONS)
				case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: {
					print_handle_result_message(
						function_name,
						"VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR",
						"The specified Video Std parameters do not adhere to the syntactic "
							"or semantic requirements of the used video compression standard, "
							"or values derived from parameters according to the "
							"rules defined by the used video compression standard "
							"do not adhere to the capabilities of the video compression standard or the implementation."
					);
				} break;
			#endif

			case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_COMPRESSION_EXHAUSTED_EXT",
					"An image creation failed because internal resources required for compression are exhausted. "
					"This must only be returned when fixed-rate compression is requested."
				);
				return evo::resultError;
			} break;

			case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT: {
				print_handle_result_message(
					function_name,
					"VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT",
					"The provided binary shader code is not compatible with this device."
				);
				return evo::resultError;
			} break;

		};

		evo::debugFatalBreak("Unknown Vulkan result code");
	};

	
}