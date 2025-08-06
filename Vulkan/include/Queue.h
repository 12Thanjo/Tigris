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



			auto createSubmitInfo(
				evo::ArrayProxy<VkSemaphore> wait_semaphore,
				evo::ArrayProxy<VkPipelineStageFlags> pipeline_stage_mask,
				evo::ArrayProxy<class CommandBuffer>
			) -> VkSubmitInfo {
				evo::debugAssert(
					wait_semaphore.size() == pipeline_stage_mask.size(),
					"Num of pipeline_stage_mask must be the same as wait_semaphore"
				);
			}



			struct SubmitInfo{
				evo::ArrayProxy<VkSemaphore> wait_semaphores;
				evo::ArrayProxy<VkPipelineStageFlags> pipeline_stage_masks;
				evo::ArrayProxy<class CommandBuffer> command_buffers;
				evo::ArrayProxy<VkSemaphore> signal_semaphores;

				SubmitInfo(
					evo::ArrayProxy<VkSemaphore> _wait_semaphores,
					evo::ArrayProxy<VkPipelineStageFlags> _pipeline_stage_masks,
					evo::ArrayProxy<class CommandBuffer> _command_buffers,
					evo::ArrayProxy<VkSemaphore> _signal_semaphores
				) : 
					wait_semaphores(_wait_semaphores),
					pipeline_stage_masks(_pipeline_stage_masks),
					command_buffers(_command_buffers),
					signal_semaphores(_signal_semaphores)
				{
					evo::debugAssert(
						this->wait_semaphores.empty() 
							|| this->wait_semaphores.size() == this->pipeline_stage_masks.size(),
						"if pipeline_stage_masks not empty, "
							"num of pipeline_stage_masks must be the same as wait_semaphores"
					);
				}
			};


			auto submit(evo::ArrayProxy<SubmitInfo> submit_infos, class Fence& fence) -> evo::Result<>;

			auto waitIdle() -> void { vkQueueWaitIdle(this->queue); }

		
		private:
			VkQueue queue = VK_NULL_HANDLE;
	};

	
}