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


	class DescriptorSetLayout{
		public:
			DescriptorSetLayout() = default;

			~DescriptorSetLayout(){ if(this->isInitialized()){ this->deinit(); } }

			DescriptorSetLayout(const DescriptorSetLayout&) = delete;
			auto operator=(const DescriptorSetLayout&) -> DescriptorSetLayout& = delete;

			DescriptorSetLayout(DescriptorSetLayout&& rhs) : 
				descriptor_set_layout(std::exchange(rhs.descriptor_set_layout, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(DescriptorSetLayout&& rhs) -> DescriptorSetLayout& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(
				VkDevice device,
				evo::ArrayProxy<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings,
				VkDescriptorSetLayoutCreateFlags flags = utils::NO_FLAGS
			) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;


				const auto create_info = VkDescriptorSetLayoutCreateInfo{
					.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext        = nullptr,
					.flags        = flags,
					.bindingCount = uint32_t(descriptor_set_layout_bindings.size()),
					.pBindings    = descriptor_set_layout_bindings.data(),
				};
				
				const VkResult result = vkCreateDescriptorSetLayout(
					this->_device, &create_info, nullptr, &this->descriptor_set_layout
				);
				return utils::checkResult(result, "vkCreateDescriptorSetLayout");
			}



			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyDescriptorSetLayout(this->_device, this->descriptor_set_layout, nullptr);
				this->descriptor_set_layout = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->descriptor_set_layout != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkDescriptorSetLayout { return this->descriptor_set_layout; }


		private:
			VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
			VkDevice _device;
	};

	
}