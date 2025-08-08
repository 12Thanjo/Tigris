////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/DescriptorPool.h"


#include "../include/DescriptorSetLayout.h"


namespace vulkan{


	auto DescriptorPool::allocateDescriptorSets(evo::ArrayProxy<DescriptorSetLayout> layouts)
	-> evo::Result<evo::SmallVector<VkDescriptorSet>> {
		auto native_layouts = evo::SmallVector<VkDescriptorSetLayout>();
		native_layouts.reserve(layouts.size());

		for(const DescriptorSetLayout& layout : layouts){
			native_layouts.emplace_back(layout.native());
		}

		return this->allocateDescriptorSets(native_layouts);
	}
	

	
}