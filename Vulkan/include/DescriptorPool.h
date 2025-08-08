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


	class DescriptorPool{
		public:
			DescriptorPool() = default;

			~DescriptorPool(){ if(this->isInitialized()){ this->deinit(); } }

			DescriptorPool(const DescriptorPool&) = delete;
			auto operator=(const DescriptorPool&) -> DescriptorPool& = delete;

			DescriptorPool(DescriptorPool&& rhs) : 
				descriptor_pool(std::exchange(rhs.descriptor_pool, VK_NULL_HANDLE)), _device(rhs._device) {}

			auto operator=(DescriptorPool&& rhs) -> DescriptorPool& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(
				VkDevice device, uint32_t max_sets, evo::ArrayProxy<VkDescriptorPoolSize> pool_sizes
			) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;

				const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
					.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.pNext         = nullptr,
					.flags         = utils::NO_FLAGS,
					.maxSets       = max_sets,
					.poolSizeCount = uint32_t(pool_sizes.size()),
					.pPoolSizes    = pool_sizes.data(),
				};
				
				const VkResult result = vkCreateDescriptorPool(
					this->_device, &descriptor_pool_info, nullptr, &this->descriptor_pool
				);
				return utils::checkResult(result, "vkCreateDescriptorPool");
			}



			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyDescriptorPool(this->_device, this->descriptor_pool, nullptr);
				this->descriptor_pool = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto allocateDescriptorSets(evo::ArrayProxy<VkDescriptorSetLayout> layouts)
			-> evo::Result<evo::SmallVector<VkDescriptorSet>> {
				auto output = evo::SmallVector<VkDescriptorSet>(layouts.size());

				const auto alloc_info = VkDescriptorSetAllocateInfo{
					.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.pNext              = nullptr,
					.descriptorPool     = this->descriptor_pool,
					.descriptorSetCount = uint32_t(layouts.size()),
					.pSetLayouts        = layouts.data(),
				};
				
				const VkResult result = vkAllocateDescriptorSets(this->_device, &alloc_info, output.data());
				if(utils::checkResult(result, "vkAllocateDescriptorSets").isError()){ return evo::resultError; }

				return output;
			}

			EVO_NODISCARD auto allocateDescriptorSets(evo::ArrayProxy<class DescriptorSetLayout> layouts)
				-> evo::Result<evo::SmallVector<VkDescriptorSet>>;




			EVO_NODISCARD auto isInitialized() const -> bool { return this->descriptor_pool != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkDescriptorPool { return this->descriptor_pool; }


		private:
			VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
			VkDevice _device = VK_NULL_HANDLE;
	};

	
}