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


	class Buffer{
		public:
			Buffer() = default;

			~Buffer(){ if(this->isInitialized()){ this->deinit(); } }


			Buffer(const Buffer&) = delete;
			auto operator=(const Buffer&) = delete;

			Buffer(Buffer&& rhs) : 
				buffer(std::exchange(rhs.buffer, this->buffer)), 
				memory(std::exchange(rhs.memory, this->memory)),
				_device(std::exchange(rhs._device, this->_device)),
				_size(std::exchange(rhs._size, this->_size))
			{}

			auto operator=(Buffer&& rhs) -> Buffer& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}


			auto init(
				VkDevice device,
				VkPhysicalDevice physical_device,
				VkBufferUsageFlags usage_flags,
				VkMemoryPropertyFlags memory_property_flags,
				size_t buffer_size
			) -> evo::Result<>;
			
			auto deinit() -> void;


			EVO_NODISCARD auto writeData(evo::ArrayProxy<std::byte> data) -> evo::Result<>;

			template<class Elem>
			EVO_NODISCARD auto writeData(evo::ArrayProxy<Elem> data) -> evo::Result<> {
				return this->writeData(
					evo::ArrayProxy<std::byte>((std::byte*)data.data(), data.size() * sizeof(Elem))
				);
			}

			auto flushWrite() -> void; // needed after write if not coherent


			template<class Elem>
			EVO_NODISCARD auto extractData() -> evo::SmallVector<Elem> {
				void* mapped = this->extract_data_impl();

				auto mapped_range = evo::ArrayProxy<Elem>((Elem*)mapped, this->size() / sizeof(Elem));
				auto output = evo::SmallVector<Elem>(mapped_range.begin(), mapped_range.end());

				this->ummap_memory();

				return output;
			}

			EVO_NODISCARD auto extractData() -> evo::SmallVector<std::byte> {
				return this->extractData<std::byte>();
			}


			EVO_NODISCARD auto size() const -> size_t { return this->_size; }


			EVO_NODISCARD auto isInitialized() const -> bool { return this->buffer != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkBuffer { return this->buffer; }


		private:
			auto write_data(evo::ArrayProxy<std::byte> data) -> evo::Result<>;

			auto extract_data_impl() -> void*;
			auto ummap_memory() -> void;


		
		private:
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;

			VkDevice _device = VK_NULL_HANDLE; // TODO(FUTURE): should this stay here?

			size_t _size = 0;
	};

	
}