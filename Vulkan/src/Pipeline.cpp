////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/Pipeline.h"

#include "../include/PipelineLayout.h"
#include "../include/PipelineCache.h"


namespace vulkan{



	auto Pipeline::initCompute(
		VkDevice device,
		VkPipelineShaderStageCreateInfo stage,
		const PipelineLayout& pipeline_layout,
		const PipelineCache& pipeline_cache
	) -> evo::Result<> {
		evo::debugAssert(this->isInitialized() == false, "Already initialized");

		this->_device = device;

		const auto compute_pipeline_create_info = VkComputePipelineCreateInfo{
			.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext              = nullptr,
			.flags              = utils::NO_FLAGS,
			.stage              = stage,
			.layout             = pipeline_layout.native(),
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex  = 0,
		};

		const VkResult result = vkCreateComputePipelines(
			this->_device, pipeline_cache.native(), 1, &compute_pipeline_create_info, nullptr, &this->pipeline
		);
		return utils::checkResult(result, "vkCreateComputePipelines");
	}

	
}