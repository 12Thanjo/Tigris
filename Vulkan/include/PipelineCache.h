////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <filesystem>

#include <Evo.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <Volk/volk.h>

#include "./utils.h"



namespace vulkan{


	class PipelineCache{
		public:
			PipelineCache() = default;

			~PipelineCache(){ if(this->isInitialized()){ this->deinit(); } }

			PipelineCache(const PipelineCache&) = delete;
			auto operator=(const PipelineCache&) -> PipelineCache& = delete;

			PipelineCache(PipelineCache&& rhs) : 
				pipeline_cache(std::exchange(rhs.pipeline_cache, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(PipelineCache&& rhs) -> PipelineCache& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(VkDevice device) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;


				const auto create_info = VkPipelineCacheCreateInfo{
					.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
					.pNext           = nullptr,
					.flags           = utils::NO_FLAGS,
					.initialDataSize = 0,
					.pInitialData    = nullptr,
				};

				const VkResult result = 
					vkCreatePipelineCache(this->_device, &create_info, nullptr, &this->pipeline_cache);
				return utils::checkResult(result, "vkCreatePipelineCache");
			}


			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyPipelineCache(this->_device, this->pipeline_cache, nullptr);
				this->pipeline_cache = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->pipeline_cache != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkPipelineCache { return this->pipeline_cache; }


		private:
			VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
			VkDevice _device;
	};

	
}