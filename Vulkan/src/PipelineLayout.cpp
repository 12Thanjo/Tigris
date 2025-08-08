////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/PipelineLayout.h"


#include "../include/DescriptorSetLayout.h"


namespace vulkan{



	auto PipelineLayout::init(
		VkDevice device,
		evo::ArrayProxy<DescriptorSetLayout> descriptor_set_layouts,
		evo::ArrayProxy<VkPushConstantRange> push_constant_ranges,
		VkPipelineLayoutCreateFlags flags
	) -> evo::Result<> {
		auto native_layouts = evo::SmallVector<VkDescriptorSetLayout>();
		native_layouts.reserve(descriptor_set_layouts.size());

		for(const DescriptorSetLayout& layout : descriptor_set_layouts){
			native_layouts.emplace_back(layout.native());
		}

		return this->init(device, native_layouts, push_constant_ranges, flags);
	}
	

	
}