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


	class Pipeline{
		public:
			Pipeline() = default;

			~Pipeline(){ if(this->isInitialized()){ this->deinit(); } }

			Pipeline(const Pipeline&) = delete;
			auto operator=(const Pipeline&) -> Pipeline& = delete;

			Pipeline(Pipeline&& rhs) : 
				pipeline(std::exchange(rhs.pipeline, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(Pipeline&& rhs) -> Pipeline& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto initCompute(
				VkDevice device,
				VkPipelineShaderStageCreateInfo stage,
				const class PipelineLayout& pipeline_layout,
				const class PipelineCache& pipeline_cache
			) -> evo::Result<>;



			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyPipeline(this->_device, this->pipeline, nullptr);
				this->pipeline = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->pipeline != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkPipeline { return this->pipeline; }


		private:
			VkPipeline pipeline = VK_NULL_HANDLE;
			VkDevice _device;
	};

	
}