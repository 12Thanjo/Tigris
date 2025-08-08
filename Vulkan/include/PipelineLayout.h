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


	class PipelineLayout{
		public:
			PipelineLayout() = default;

			~PipelineLayout(){ if(this->isInitialized()){ this->deinit(); } }

			PipelineLayout(const PipelineLayout&) = delete;
			auto operator=(const PipelineLayout&) -> PipelineLayout& = delete;

			PipelineLayout(PipelineLayout&& rhs) : 
				pipeline_layout(std::exchange(rhs.pipeline_layout, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(PipelineLayout&& rhs) -> PipelineLayout& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(
				VkDevice device,
				evo::ArrayProxy<VkDescriptorSetLayout> descriptor_set_layouts,
				evo::ArrayProxy<VkPushConstantRange> push_constant_ranges,
				VkPipelineLayoutCreateFlags flags = utils::NO_FLAGS
			) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;


				const auto pipeline_layout_create_info = VkPipelineLayoutCreateInfo{
					.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
					.pNext                  = nullptr,
					.flags                  = flags,
					.setLayoutCount         = uint32_t(descriptor_set_layouts.size()),
					.pSetLayouts            = descriptor_set_layouts.data(),
					.pushConstantRangeCount = uint32_t(push_constant_ranges.size()),
					.pPushConstantRanges    = push_constant_ranges.data(),
				};
				
				const VkResult result = vkCreatePipelineLayout(
					this->_device, &pipeline_layout_create_info, nullptr, &this->pipeline_layout
				);
				return utils::checkResult(result, "vkCreatePipelineLayout");
			}

			EVO_NODISCARD auto init(
				VkDevice device,
				evo::ArrayProxy<class DescriptorSetLayout> descriptor_set_layouts,
				evo::ArrayProxy<VkPushConstantRange> push_constant_ranges,
				VkPipelineLayoutCreateFlags flags = utils::NO_FLAGS
			) -> evo::Result<>;



			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyPipelineLayout(this->_device, this->pipeline_layout, nullptr);
				this->pipeline_layout = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->pipeline_layout != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkPipelineLayout { return this->pipeline_layout; }


		private:
			VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
			VkDevice _device;
	};

	
}