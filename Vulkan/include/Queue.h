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

#include "./utils.h"


namespace vulkan{


	class Queue{
		public:
			Queue() = default;
			Queue(VkQueue _queue) : queue(_queue) {}
			Queue(VkDevice device, uint32_t queue_family_index, uint32_t queue_index) {
				vkGetDeviceQueue(device, queue_family_index, queue_index, &this->queue);
			}

			~Queue() = default;


			EVO_NODISCARD auto isInitialized() const -> bool { return this->queue != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkQueue { return this->queue; }


			auto submit(evo::ArrayProxy<VkSubmitInfo> submit_infos, VkFence fence) -> evo::Result<> {
				const VkResult result = 
					vkQueueSubmit(this->queue, uint32_t(submit_infos.size()), submit_infos.data(), fence);
				if(utils::checkResult(result, "vkQueueSubmit").isError()){ return evo::resultError; }
				return evo::Result<>();
			}

			auto waitIdle() -> void { vkQueueWaitIdle(this->queue); }

		
		// TODO(FUTURE): make private
		public:
			VkQueue queue = VK_NULL_HANDLE;
	};

	
}