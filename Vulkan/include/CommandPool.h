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

#include "./CommandBuffer.h"


namespace vulkan{


	class CommandPool{
		public:
			CommandPool() = default;

			~CommandPool(){ if(this->isInitialized()){ this->deinit(); } }

			CommandPool(const CommandPool&) = delete;


			EVO_NODISCARD auto init(VkDevice device, uint32_t queue_family_index) -> evo::Result<>;
			
			auto deinit() -> void;


			EVO_NODISCARD auto allocateCommandBuffers(size_t num, bool is_primary = true)
				-> evo::Result<evo::SmallVector<CommandBuffer>>;



			EVO_NODISCARD auto isInitialized() const -> bool { return this->cmd_pool != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkCommandPool { return this->cmd_pool; }

		
		private:
			VkCommandPool cmd_pool = VK_NULL_HANDLE;

			VkDevice _device = VK_NULL_HANDLE;

			friend class CommandBuffer;
	};

	
}