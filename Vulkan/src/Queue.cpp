////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/Queue.h"


#include "../include/CommandBuffer.h"
#include "../include/Fence.h"


namespace vulkan{



	auto Queue::submit(evo::ArrayProxy<SubmitInfo> submit_infos, Fence& fence) -> evo::Result<> {
		auto native_submit_infos = evo::SmallVector<VkSubmitInfo, 2>();
		native_submit_infos.reserve(submit_infos.size());

		auto native_command_buffers_list = evo::SmallVector<evo::SmallVector<VkCommandBuffer>, 4>();



		for(const SubmitInfo& submit_info : submit_infos){
			evo::SmallVector<VkCommandBuffer>& native_command_buffers = native_command_buffers_list.emplace_back();

			for(const CommandBuffer& command_buffer : submit_info.command_buffers){
				native_command_buffers.emplace_back(command_buffer.native());
			}

			native_submit_infos.emplace_back(
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				nullptr,
				uint32_t(submit_info.wait_semaphores.size()),
				submit_info.wait_semaphores.data(),
				submit_info.pipeline_stage_masks.data(),
				uint32_t(native_command_buffers.size()),
				native_command_buffers.data(),
				uint32_t(submit_info.signal_semaphores.size()),
				submit_info.signal_semaphores.data()
			);
		}


		const VkResult result = vkQueueSubmit(
			this->queue, uint32_t(native_submit_infos.size()), native_submit_infos.data(), fence.native()
		);
		return utils::checkResult(result, "vkQueueSubmit");
	}

	
}