////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/CommandPool.h"



namespace vulkan{


	auto CommandPool::init(VkDevice device, uint32_t queue_family_index) -> evo::Result<> {
		evo::debugAssert(this->isInitialized() == false, "Already initialized");

		this->_device = device;

		const auto create_info = VkCommandPoolCreateInfo{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext            = nullptr,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queue_family_index,
		};

		const VkResult result = vkCreateCommandPool(device, &create_info, nullptr, &this->cmd_pool);
		if(utils::checkResult(result, "vkCreateCommandPool").isError()){ return evo::resultError; }

		return evo::Result<>();
	}


	auto CommandPool::deinit() -> void {
		evo::debugAssert(this->isInitialized(), "Not initialized");

		vkDestroyCommandPool(this->_device, this->cmd_pool, nullptr);
		this->cmd_pool = VK_NULL_HANDLE;
	}



	auto CommandPool::allocateCommandBuffers(size_t num, bool is_primary)
	-> evo::Result<evo::SmallVector<CommandBuffer>> {
		evo::debugAssert(num != 0, "Cannot allocate 0 command buffers");

		const auto allocate_info = VkCommandBufferAllocateInfo{
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext              = nullptr,
			.commandPool        = this->cmd_pool,
			.level              = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = uint32_t(num),
		};

		auto command_buffers = evo::SmallVector<VkCommandBuffer>(num, VK_NULL_HANDLE);
		const VkResult result = vkAllocateCommandBuffers(this->_device, &allocate_info, command_buffers.data());
		if(utils::checkResult(result, "vkAllocateCommandBuffers").isError()){ return evo::resultError; }


		auto output = evo::Result<evo::SmallVector<CommandBuffer>>(evo::SmallVector<CommandBuffer>());
		output.value().reserve(num);
		for(VkCommandBuffer command_buffer : command_buffers){
			output.value().emplace_back(*this, command_buffer);
		}

		return output;
	}

	
}