////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/Buffer.h"



namespace vulkan{


	auto Buffer::init(
		VkDevice device,
		VkPhysicalDevice physical_device,
		VkBufferUsageFlags usage_flags,
		VkMemoryPropertyFlags memory_property_flags,
		size_t buffer_size
	) -> evo::Result<> {
		evo::debugAssert(this->isInitialized() == false, "Already initialized");

		this->_size = buffer_size;
		this->_device = device;

		const auto buffer_create_info = VkBufferCreateInfo{
			.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext                 = nullptr,
			.flags                 = utils::NO_FLAGS,
			.size                  = static_cast<VkDeviceSize>(this->size()),
			.usage                 = usage_flags,
			.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr,
		};

		const VkResult buffer_create_result = vkCreateBuffer(device, &buffer_create_info, nullptr, &this->buffer);
		if(utils::checkResult(buffer_create_result, "vkCreateBuffer").isError()){ return evo::resultError; }


		//////////////////
		// find a memory type index that fits the properties of the buffer

		const auto buffer_memory_requirements_info = VkBufferMemoryRequirementsInfo2{
			.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
			.pNext  = nullptr,
			.buffer = this->buffer,
		};
		auto mem_reqs = VkMemoryRequirements2{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };  // uninit
		vkGetBufferMemoryRequirements2(device, &buffer_memory_requirements_info, &mem_reqs);


		auto device_memory_properties = VkPhysicalDeviceMemoryProperties2{ // uninit
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
		};
		vkGetPhysicalDeviceMemoryProperties2(physical_device, &device_memory_properties);

		const evo::Result<VkMemoryAllocateInfo> mem_alloc = [&]() -> evo::Result<VkMemoryAllocateInfo> {
			for(size_t i = 0; i < device_memory_properties.memoryProperties.memoryTypeCount; i+=1){
				if((mem_reqs.memoryRequirements.memoryTypeBits & 1) == 1){
					if(
						(device_memory_properties.memoryProperties.memoryTypes[i].propertyFlags & memory_property_flags)
						== memory_property_flags
					){
						return VkMemoryAllocateInfo{
							.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
							.pNext           = nullptr,
							.allocationSize  = mem_reqs.memoryRequirements.size,
							.memoryTypeIndex = uint32_t(i),
						};
					}
				}
				mem_reqs.memoryRequirements.memoryTypeBits >>= 1;
			}

			evo::log::error("No required Vulkan memory type index found");
			return evo::resultError;
		}();
		if(mem_alloc.isError()){ return evo::resultError; }


		//////////////////
		// create buffer memory
		
		const VkResult allocate_result = vkAllocateMemory(device, &mem_alloc.value(), nullptr, &this->memory);
		if(utils::checkResult(allocate_result, "vkAllocateMemory").isError()){ return evo::resultError; }

		const VkResult bind_result = vkBindBufferMemory(device, this->buffer, this->memory, 0);
		if(utils::checkResult(bind_result, "vkBindBufferMemory").isError()){ return evo::resultError; }


		//////////////////
		// done

		return evo::Result<>();
	}


	auto Buffer::deinit() -> void {
		evo::debugAssert(this->isInitialized(), "Not initialized");

		vkDestroyBuffer(this->_device, this->buffer, nullptr);
		vkFreeMemory(this->_device, this->memory, nullptr);

		this->buffer = VK_NULL_HANDLE;

		#if defined(_DEBUG)
			this->memory = VK_NULL_HANDLE;
			this->_device = VK_NULL_HANDLE;
			this->_size = 0;
		#endif
	}



	auto Buffer::writeData(evo::ArrayProxy<std::byte> data) -> evo::Result<> {
		evo::debugAssert(this->isInitialized(), "Not initialized");
		evo::debugAssert(this->size() == data.size(), "Incorrect sized data for this buffer");

		void* mapped;
		const VkResult map_result = vkMapMemory(
			this->_device, this->memory, 0, static_cast<VkDeviceSize>(this->size()), 0, &mapped
		);
		if(utils::checkResult(map_result, "vkMapMemory").isError()){ return evo::resultError; }

		std::memcpy(mapped, data.data(), data.size());

		vkUnmapMemory(this->_device, this->memory);

		return evo::Result<>();
	}


	auto Buffer::flushWrite() -> void {
		void* mapped;
		vkMapMemory(this->_device, this->memory, 0, VK_WHOLE_SIZE, 0, &mapped);
		const auto mapped_range = VkMappedMemoryRange{
			.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			.pNext  = nullptr,
			.memory = this->memory,
			.offset = 0,
			.size   = VK_WHOLE_SIZE,
		};
		vkFlushMappedMemoryRanges(this->_device, 1, &mapped_range);
		vkUnmapMemory(this->_device, this->memory);
	}


	auto Buffer::extract_data_impl() -> void* {
		void* mapped;
		vkMapMemory(this->_device, this->memory, 0, VK_WHOLE_SIZE, 0, &mapped);
		const auto mapped_range = VkMappedMemoryRange{
			.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			.pNext  = nullptr,
			.memory = this->memory,
			.offset = 0,
			.size   = VK_WHOLE_SIZE,
		};
		vkInvalidateMappedMemoryRanges(this->_device, 1, &mapped_range);

		return mapped;
	}

	auto Buffer::ummap_memory() -> void {
		vkUnmapMemory(this->_device, this->memory);
	}


	
}