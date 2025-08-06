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


	class CommandBuffer{
		public:
			CommandBuffer(class CommandPool& command_pool, VkCommandBuffer command_buffer)
				: cmd_buffer(command_buffer), cmd_pool(&command_pool) {}

			#if defined(_DEBUG)
				~CommandBuffer(){ evo::debugAssert(this->isInitialized() == false, "Should have been deinitialized"); }
			#else
				~CommandBuffer() = default;
			#endif

			CommandBuffer(const CommandBuffer&) = delete;
			auto operator=(const CommandBuffer&) -> CommandBuffer& = delete;

			CommandBuffer(CommandBuffer&& rhs) :
				cmd_buffer(std::exchange(rhs.cmd_buffer, VK_NULL_HANDLE)),
				cmd_pool(std::exchange(rhs.cmd_pool, nullptr))
			{}

			auto operator=(CommandBuffer&& rhs) -> CommandBuffer& {
				if(this->isInitialized()){ this->deinit(); }
				this->cmd_buffer = rhs.cmd_buffer;
				this->cmd_pool = rhs.cmd_pool;

				rhs.cmd_buffer = VK_NULL_HANDLE;
				rhs.cmd_pool = nullptr;

				return *this;
			}


			auto deinit() -> void;

			EVO_NODISCARD auto isInitialized() const -> bool { return this->cmd_buffer != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkCommandBuffer { return this->cmd_buffer; }


			///////////////////////////////////
			// commands

			auto begin(VkCommandBufferUsageFlags flags = utils::NO_FLAGS) -> evo::Result<>;
			auto end() -> evo::Result<>;

			auto copyBuffer(const class Buffer& src, class Buffer& dst) -> void;

			auto pipelineBarrier(
				VkPipelineStageFlags src_stage_mask,
				VkPipelineStageFlags dst_stage_mask,
				VkDependencyFlags dependency_flags,
				evo::ArrayProxy<VkMemoryBarrier> memory_barriers,
				evo::ArrayProxy<VkBufferMemoryBarrier> buffer_memory_barriers,
				evo::ArrayProxy<VkImageMemoryBarrier> image_memory_barriers
			) -> void;

			auto bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point) -> void;

			auto bindDescriptorSets(
				VkPipelineBindPoint pipeline_bind_point,
				VkPipelineLayout layout,
				uint32_t first_set,
				evo::ArrayProxy<VkDescriptorSet> descriptor_sets,
				evo::ArrayProxy<uint32_t> dynamic_offsets
			) -> void;

			auto dispatch(uint32_t count_x, uint32_t count_y = 1, uint32_t count_z = 1) -> void;



		private:
			VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
			CommandPool* cmd_pool = nullptr;

	};

	
}