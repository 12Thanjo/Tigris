////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Evo.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <Volk/volk.h>

#include "./Queue.h"


namespace vulkan{


	class Engine{
		public:
			Engine() = default;

			#if defined(_DEBUG)
				~Engine(){ evo::debugAssert(this->isInitialized() == false, "Should have been deinitialized"); }
			#else
				~Engine() = default;
			#endif

			auto init() -> evo::Result<>;
			auto deinit() -> void;

			EVO_NODISCARD auto isInitialized() const -> bool;


		private:
			auto deinit_due_to_fail_to_initailize() -> void;

			auto create_instance() -> evo::Result<>;
			auto select_physical_device() -> evo::Result<>;
			auto create_device_and_queue() -> evo::Result<>;

			auto print_physical_device_properties(VkPhysicalDevice target_physical_device) const -> void;

		
		// TODO(FUTURE): make private
		public:
			VkInstance instance = VK_NULL_HANDLE;
			#if defined(_DEBUG)
				VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
			#endif

			VkPhysicalDevice physical_device = VK_NULL_HANDLE;
			uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();
			VkDevice device = VK_NULL_HANDLE;
			Queue queue{};
	};

	
}