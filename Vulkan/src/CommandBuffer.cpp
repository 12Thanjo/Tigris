////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/CommandBuffer.h"


#include "../include/Buffer.h"
#include "../include/CommandPool.h"


namespace vulkan{


	auto CommandBuffer::deinit() -> void {
		vkFreeCommandBuffers(this->cmd_pool->_device, this->cmd_pool->native(), 1, &this->cmd_buffer);
		this->cmd_buffer = VK_NULL_HANDLE;
		this->cmd_pool = nullptr;
	}



	auto CommandBuffer::begin(VkCommandBufferUsageFlags flags) -> evo::Result<> {
		const auto command_buffer_begin_info = VkCommandBufferBeginInfo{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext            = nullptr,
			.flags            = flags,
			.pInheritanceInfo = nullptr,
		};

		const VkResult result = vkBeginCommandBuffer(this->cmd_buffer, &command_buffer_begin_info);
		return utils::checkResult(result, "vkBeginCommandBuffer");
	}

	auto CommandBuffer::end() -> evo::Result<> {
		return utils::checkResult(vkEndCommandBuffer(this->cmd_buffer), "vkEndCommandBuffer");
	}



	auto CommandBuffer::copyBuffer(const Buffer& src, Buffer& dst) -> void {
		evo::debugAssert(src.size() == dst.size(), "Buffers are differing sizes");

		const auto copy_region = VkBufferCopy{
			.srcOffset = 0,
			.dstOffset = 0,
			.size      = src.size(),
		};
		vkCmdCopyBuffer(this->cmd_buffer, src.native(), dst.native(), 1, &copy_region);
	}



	auto CommandBuffer::pipelineBarrier(
		VkPipelineStageFlags src_stage_mask,
		VkPipelineStageFlags dst_stage_mask,
		VkDependencyFlags dependency_flags,
		evo::ArrayProxy<VkMemoryBarrier> memory_barriers,
		evo::ArrayProxy<VkBufferMemoryBarrier> buffer_memory_barriers,
		evo::ArrayProxy<VkImageMemoryBarrier> image_memory_barriers
	) -> void {
		vkCmdPipelineBarrier(
			this->cmd_buffer,
			src_stage_mask,
			dst_stage_mask,
			dependency_flags,
			uint32_t(memory_barriers.size()),
			memory_barriers.data(),
			uint32_t(buffer_memory_barriers.size()),
			buffer_memory_barriers.data(),
			uint32_t(image_memory_barriers.size()),
			image_memory_barriers.data()
		);
	}


	auto CommandBuffer::bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point) -> void {
		vkCmdBindPipeline(this->cmd_buffer, bind_point, pipeline);
	}

	auto CommandBuffer::bindDescriptorSets(
		VkPipelineBindPoint pipeline_bind_point,
		VkPipelineLayout layout,
		uint32_t first_set,
		evo::ArrayProxy<VkDescriptorSet> descriptor_sets,
		evo::ArrayProxy<uint32_t> dynamic_offsets
	) -> void {
		vkCmdBindDescriptorSets(
			this->cmd_buffer,
			pipeline_bind_point,
			layout,
			first_set,
			uint32_t(descriptor_sets.size()),
			descriptor_sets.data(),
			uint32_t(dynamic_offsets.size()),
			dynamic_offsets.data()
		);
	}


	auto CommandBuffer::dispatch(uint32_t count_x, uint32_t count_y, uint32_t count_z) -> void {
		vkCmdDispatch(this->cmd_buffer, count_x, count_y, count_z);
	}

	
}