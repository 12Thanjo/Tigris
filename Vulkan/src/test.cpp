////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/test.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <Volk/volk.h>

#include <filesystem>

namespace vulkan{

	static constexpr int VULKAN_NO_FLAGS = 0;

	EVO_NODISCARD static auto select_physical_device(evo::ArrayProxy<VkPhysicalDevice> physical_devices)
	-> evo::Result<VkPhysicalDevice> {
		for(const VkPhysicalDevice& physical_device : physical_devices){
			auto props = VkPhysicalDeviceProperties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
			vkGetPhysicalDeviceProperties2(physical_device, &props);

			if(props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
				evo::println("Using discrete GPU: \"{}\"", props.properties.deviceName);
				return physical_device;
			}
		}

		if(physical_devices.empty()){
			evo::log::fatal("No physical devices available for Vulkan");
			return evo::resultError;
		}

		auto props = VkPhysicalDeviceProperties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		vkGetPhysicalDeviceProperties2(physical_devices[0], &props);
		evo::println("No discrete GPU detected");
		evo::println("Using fallback GPU: \"{}\"", props.properties.deviceName);
		return physical_devices[0];
	}


	EVO_NODISCARD static auto select_queue_family_index(VkPhysicalDevice physical_device) -> evo::Result<uint32_t> {
		uint32_t num_queue_families;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);

		auto queue_family_props = std::vector<VkQueueFamilyProperties>(num_queue_families);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, queue_family_props.data());

		for(size_t i = 0; const VkQueueFamilyProperties& queue_family_prop : queue_family_props){
			if((queue_family_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0){
				return uint32_t(i);
			}
		
			i += 1;
		}

		evo::log::fatal("physical device doesn't support Vulkan compute");
		return evo::resultError;
	}



	EVO_NODISCARD static auto create_buffer(
		VkDevice device,
		VkPhysicalDevice physical_device,
		VkBufferUsageFlags usage_flags,
		VkMemoryPropertyFlags memory_property_flags,
		VkBuffer* buffer,
		VkDeviceMemory* memory,
		VkDeviceSize size,
		void* data = nullptr
	) -> evo::Result<> {

		//////////////////
		// create buffer handle

		const auto buffer_create_info = VkBufferCreateInfo{
			.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext                 = nullptr,
			.flags                 = VULKAN_NO_FLAGS,
			.size                  = size,
			.usage                 = usage_flags,
			.sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices   = nullptr,
		};
		if(vkCreateBuffer(device, &buffer_create_info, nullptr, buffer) != VK_SUCCESS){
			evo::log::fatal("Failed to create vulkan buffer");
			return evo::resultError;
		}


		//////////////////
		// create buffer memory

		const auto buffer_memory_requirements_info = VkBufferMemoryRequirementsInfo2{
			.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
			.pNext  = nullptr,
			.buffer = *buffer,
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

			evo::log::fatal("No required Vulkan memory type index found");
			return evo::resultError;
		}();
		if(mem_alloc.isError()){ return evo::resultError; }


		//////////////////
		// find a memory type index that fits the properties of the buffer

		if(vkAllocateMemory(device, &mem_alloc.value(), nullptr, memory) != VK_SUCCESS){
			evo::log::fatal("Failed to allocate memory for Vulkan buffer");
			return evo::resultError;
		}

		if(data != nullptr){
			void* mapped;
			if(vkMapMemory(device, *memory, 0, size, 0, &mapped) != VK_SUCCESS){
				evo::log::fatal("Failed to map Vulkan memory");
				return evo::resultError;
			}

			std::memcpy(mapped, data, size);

			vkUnmapMemory(device, *memory);
		}

		if(vkBindBufferMemory(device, *buffer, *memory, 0) != VK_SUCCESS){
			evo::log::fatal("Failed to bind Vulkan buffer memory");
			return evo::resultError;
		}


		//////////////////
		// done

		return evo::Result<>();
	}



	EVO_NODISCARD static auto create_shader_module(VkDevice device, const std::filesystem::path& path)
	-> evo::Result<VkShaderModule> {
		const evo::Result<std::vector<evo::byte>> file = evo::fs::readBinaryFile(path.string());
		if(file.isError()){
			evo::log::fatal("Failed to load shader at path: \"{}\"", path.string());
			return evo::resultError;
		}

		if(file.value().size() % 4 != 0){
			evo::log::fatal("File \"{}\" is not a valid shader file", path.string());
			return evo::resultError;
		}

		if(reinterpret_cast<const uint32_t*>(file.value().data())[0] != 0x07230203){
			evo::log::fatal("File \"{}\" is not a valid shader file", path.string());
			return evo::resultError;
		}

		const auto create_info = VkShaderModuleCreateInfo{
			.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext    = nullptr,
			.flags    = VULKAN_NO_FLAGS,
			.codeSize = file.value().size(),
			.pCode    = reinterpret_cast<const uint32_t*>(file.value().data()),
		};

		VkShaderModule output;
		if(vkCreateShaderModule(device, &create_info, nullptr, &output) != VK_SUCCESS){
			evo::log::fatal("Failed to create shader at path: \"{}\"", path.string());
			return evo::resultError;
		}

		return output;
	}




	auto test() -> evo::Result<> {
		evo::log::debug("Vulkan Test");


		if(volkInitialize() != VK_SUCCESS){
			evo::log::fatal("Your computer doesn't support Vulkan (failed to initialize Volk)");
			return evo::Result<>();
		}


		///////////////////////////////////
		// create instance

		const auto app_info = VkApplicationInfo{
			.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext              = nullptr,
			.pApplicationName   = "Tigris",
			.applicationVersion = VK_MAKE_API_VERSION(0, 0, 3, 0),
			.pEngineName        = "Tigris Engine",
			.engineVersion      = VK_MAKE_API_VERSION(0, 0, 3, 0),
			.apiVersion         = VK_API_VERSION_1_1,
		};


		const auto debug_layers = std::to_array<const char*>({
			"VK_LAYER_KHRONOS_validation",
		});

		const auto instance_create_info = VkInstanceCreateInfo{
			.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext                   = nullptr,
			.flags                   = VULKAN_NO_FLAGS,
			.pApplicationInfo        = &app_info,

			#if defined(_DEBUG)
				.enabledLayerCount   = uint32_t(debug_layers.size()),
				.ppEnabledLayerNames = debug_layers.data(),
			#else
				.enabledLayerCount   = 0,
				.ppEnabledLayerNames = nullptr,
			#endif

			.enabledExtensionCount   = 0,
			.ppEnabledExtensionNames = nullptr,
		};

		VkInstance instance;
		if(vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS){
			evo::log::fatal("Your computer doesn't support Vulkan (failed to create Vulkan instance)");
			return evo::resultError;
		}

		volkLoadInstance(instance);


		///////////////////////////////////
		// create device

		uint32_t num_physical_devices = 0;
		vkEnumeratePhysicalDevices(instance, &num_physical_devices, nullptr);

		auto physical_devices = std::vector<VkPhysicalDevice>(num_physical_devices);
		vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices.data());

		const evo::Result<VkPhysicalDevice> physical_device = select_physical_device(physical_devices);
		if(physical_device.isError()){ return evo::resultError; }


		const auto queue_priorities = std::array<float, 1>{1.0f};

		const evo::Result<uint32_t> queue_family_index = select_queue_family_index(physical_device.value());
		if(queue_family_index.isError()){ return evo::resultError; }

		const auto queue_create_info = VkDeviceQueueCreateInfo{
			.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext            = nullptr,
			.flags            = VULKAN_NO_FLAGS,
			.queueFamilyIndex = queue_family_index.value(),
			.queueCount       = uint32_t(queue_priorities.size()),
			.pQueuePriorities = queue_priorities.data(),
		};

		const auto device_create_info = VkDeviceCreateInfo{
			.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext                   = nullptr,
			.flags                   = VULKAN_NO_FLAGS,
			.queueCreateInfoCount    = 1,
			.pQueueCreateInfos       = &queue_create_info,
			.enabledLayerCount       = 0,       // depricated
			.ppEnabledLayerNames     = nullptr, // depricated
			.enabledExtensionCount   = 0,
			.ppEnabledExtensionNames = nullptr,
			.pEnabledFeatures        = nullptr,
		};

		VkDevice device;
		if(vkCreateDevice(physical_device.value(), &device_create_info, nullptr, &device) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan device");
			return evo::resultError;
		}


		///////////////////////////////////
		// create single compute queue

		VkQueue queue;
		vkGetDeviceQueue(device, queue_family_index.value(), 0, &queue);


		///////////////////////////////////
		// create command pool

		const auto command_pool_create_info = VkCommandPoolCreateInfo{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext            = nullptr,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queue_family_index.value(),
		};
		VkCommandPool command_pool;
		if(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan command pool");
			return evo::resultError;
		}


		///////////////////////////////////
		// prepare storage buffers

		static constexpr size_t NUM_BUFFER_ELEMS = 60;

		auto compute_input = std::vector<uint32_t>(NUM_BUFFER_ELEMS);
		auto compute_output = std::vector<uint32_t>(NUM_BUFFER_ELEMS);

		for(size_t i = 0; i < NUM_BUFFER_ELEMS; i+=1){
			compute_input[i] = uint32_t(i);
		}

		static constexpr VkDeviceSize BUFFER_SIZE = NUM_BUFFER_ELEMS * sizeof(uint32_t);


		VkBuffer device_buffer;
		VkDeviceMemory device_memory;

		VkBuffer host_buffer;
		VkDeviceMemory host_memory;


		///////////////////////////////////
		// copy input data to VRAM iusing a staging buffer
		{
			if(create_buffer(
				device,
				physical_device.value(),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				&host_buffer,
				&host_memory,
				BUFFER_SIZE,
				compute_input.data()
			).isError()){ return evo::resultError; }


			//////////////////
			// flush writes to host visible buffer

			void* mapped;
			vkMapMemory(device, host_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
			const auto mapped_range = VkMappedMemoryRange{
				.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.pNext  = nullptr,
				.memory = host_memory,
				.offset = 0,
				.size   = VK_WHOLE_SIZE,
			};
			vkFlushMappedMemoryRanges(device, 1, &mapped_range);
			vkUnmapMemory(device, host_memory);

			if(create_buffer(
				device,
				physical_device.value(),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
					| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
					| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&device_buffer,
				&device_memory,
				BUFFER_SIZE
			).isError()){ return evo::resultError; }


			//////////////////
			// copy to staging buffer

			const auto cmd_buffer_allocate_info = VkCommandBufferAllocateInfo{
				.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext              = nullptr,
				.commandPool        = command_pool,
				.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			VkCommandBuffer copy_cmd;
			if(vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info, &copy_cmd) != VK_SUCCESS){
				evo::log::fatal("Failed to allocate Vulcan command buffer");
				return evo::resultError;
			}

			const auto command_buffer_begin_info = VkCommandBufferBeginInfo{
				.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext            = nullptr,
				.flags            = VULKAN_NO_FLAGS,
				.pInheritanceInfo = nullptr,
			};
			if(vkBeginCommandBuffer(copy_cmd, &command_buffer_begin_info) != VK_SUCCESS){
				evo::log::fatal("Failed to begin Vulkan command buffer");
				return evo::resultError;
			}

			const auto copy_region = VkBufferCopy{
				.srcOffset = 0,
				.dstOffset = 0,
				.size      = BUFFER_SIZE,
			};
			vkCmdCopyBuffer(copy_cmd, host_buffer, device_buffer, 1, &copy_region);
			if(vkEndCommandBuffer(copy_cmd) != VK_SUCCESS){
				evo::log::fatal("failed to end command buffer");
				return evo::resultError;
			}

			const auto fence_create_info = VkFenceCreateInfo{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VULKAN_NO_FLAGS,
			};
			VkFence fence;
			if(vkCreateFence(device, &fence_create_info, nullptr, &fence) != VK_SUCCESS){
				evo::log::fatal("Failed to create Vulkan fence");
				return evo::resultError;
			}


			//////////////////
			// Submit to the queue

			const auto submit_info = VkSubmitInfo{
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext                = nullptr,
				.waitSemaphoreCount   = 0,
				.pWaitSemaphores      = nullptr,
				.pWaitDstStageMask    = nullptr,
				.commandBufferCount   = 1,
				.pCommandBuffers      = &copy_cmd,
				.signalSemaphoreCount = 0,
				.pSignalSemaphores    = nullptr,
			};
			if(vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS){
				evo::log::fatal("Failed to submit Vulkan queue");
				return evo::resultError;
			}
			if(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS){
				evo::log::fatal("Failed to wait for Vulkan fences");
				return evo::resultError;
			}

			vkDestroyFence(device, fence, nullptr);
			vkFreeCommandBuffers(device, command_pool, 1, &copy_cmd);
		}



		///////////////////////////////////
		// prepare compute pipeline

		VkDescriptorPool descriptor_pool;
		VkDescriptorSetLayout descriptor_set_layout;
		VkPipelineLayout pipeline_layout;
		VkDescriptorSet descriptor_set;
		VkPipelineCache pipeline_cache;
		VkShaderModule shader_module;
		VkPipeline pipeline;
		VkCommandBuffer command_buffer;
		VkFence fence;

		const auto pool_sizes = std::to_array<VkDescriptorPoolSize>({
			VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
		});
		const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext         = nullptr,
			.flags         = VULKAN_NO_FLAGS,
			.maxSets       = 1,
			.poolSizeCount = uint32_t(pool_sizes.size()),
			.pPoolSizes    = pool_sizes.data(),
		};
		if(vkCreateDescriptorPool(device, &descriptor_pool_info, nullptr, &descriptor_pool) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan descriptor pool");
			return evo::resultError;
		}


		const auto set_layout_bindings = std::to_array<VkDescriptorSetLayoutBinding>({
			VkDescriptorSetLayoutBinding{
				.binding            = 0,
				.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount    = 1,
				.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
				.pImmutableSamplers = nullptr,
			},
		});
		const auto descriptor_layout = VkDescriptorSetLayoutCreateInfo{
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext        = nullptr,
			.flags        = VULKAN_NO_FLAGS,
			.bindingCount = uint32_t(set_layout_bindings.size()),
			.pBindings    = set_layout_bindings.data(),
		};
		if(vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, &descriptor_set_layout) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan descriptor set layout");
			return evo::resultError;
		}


		const auto pipeline_create_info = VkPipelineLayoutCreateInfo{
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext                  = nullptr,
			.flags                  = VULKAN_NO_FLAGS,
			.setLayoutCount         = 1,
			.pSetLayouts            = &descriptor_set_layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges    = nullptr,
		};
		if(vkCreatePipelineLayout(device, &pipeline_create_info, nullptr, &pipeline_layout) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan pipeline layout");
			return evo::resultError;
		}


		const auto alloc_info = VkDescriptorSetAllocateInfo{
			.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext              = nullptr,
			.descriptorPool     = descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts        = &descriptor_set_layout,
		};
		if(vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) != VK_SUCCESS){
			evo::log::fatal("Failed to allocate Vulkan descriptor set");
			return evo::resultError;
		}


		const auto buffer_descriptor = VkDescriptorBufferInfo(device_buffer, 0, VK_WHOLE_SIZE);
		const auto compute_write_descriptor_sets = std::to_array<VkWriteDescriptorSet>({
			VkWriteDescriptorSet{
				.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext            = nullptr,
				.dstSet           = descriptor_set,
				.dstBinding       = 0,
				.dstArrayElement  = 0,
				.descriptorCount  = 1,
				.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pImageInfo       = nullptr,
				.pBufferInfo      = &buffer_descriptor,
				.pTexelBufferView = nullptr,
			}
		});
		vkUpdateDescriptorSets(
			device,
			uint32_t(compute_write_descriptor_sets.size()),
			compute_write_descriptor_sets.data(),
			0,
			nullptr
		);



		const auto pipline_cache_create_info = VkPipelineCacheCreateInfo{
			.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext           = nullptr,
			.flags           = VULKAN_NO_FLAGS,
			.initialDataSize = 0,
			.pInitialData    = nullptr,
		};
		if(vkCreatePipelineCache(device, &pipline_cache_create_info, nullptr, &pipeline_cache) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan pipeline cache");
			return evo::resultError;
		}


		//////////////////
		// pass SSBO size via specialiation constant

		struct SpecializationData{
			uint32_t num_buffer_elems = NUM_BUFFER_ELEMS;
		};
		const auto specialization_data = SpecializationData();
		const auto specialization_map_entry = VkSpecializationMapEntry(0, 0, sizeof(uint32_t));
		const auto specialization_info = VkSpecializationInfo{
			.mapEntryCount = 1,
			.pMapEntries   = &specialization_map_entry,
			.dataSize      = sizeof(SpecializationData),
			.pData         = &specialization_data,
		};

		const evo::Result<VkShaderModule> shader_module_res =
			create_shader_module(device, "../Vulkan/shaders/compute.comp.spv");
		if(shader_module_res.isError()){ return evo::resultError; }

		shader_module = shader_module_res.value();


		//////////////////
		// create pipeline

		const auto compute_pipeline_create_info = VkComputePipelineCreateInfo{
			.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext              = nullptr,
			.flags              = VULKAN_NO_FLAGS,
			.stage              = VkPipelineShaderStageCreateInfo{
				.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext               = nullptr,
				.flags               = VULKAN_NO_FLAGS,
				.stage               = VK_SHADER_STAGE_COMPUTE_BIT,
				.module              = shader_module,
				.pName               = "main",
				.pSpecializationInfo = &specialization_info,
			},
			.layout             = pipeline_layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex  = 0,
		};
		if(vkCreateComputePipelines(
			device, pipeline_cache, 1, &compute_pipeline_create_info, nullptr, &pipeline
		) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan compute pipeline");
			return evo::resultError;
		}

		// Create a command buffer for compute operations
		const auto cmd_buffer_allocate_info = VkCommandBufferAllocateInfo{
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext              = nullptr,
			.commandPool        = command_pool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		if(vkAllocateCommandBuffers(device, &cmd_buffer_allocate_info, &command_buffer) != VK_SUCCESS){
			evo::log::fatal("Failed to create vulkan command buffers");
			return evo::resultError;
		}

		// Fence for compute CB sync
		const auto fence_create_info = VkFenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		if(vkCreateFence(device, &fence_create_info, nullptr, &fence) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan fence");
			return evo::resultError;
		}


		///////////////////////////////////
		// create command buffer for compute work submission

		const auto command_buffer_begin_info = VkCommandBufferBeginInfo{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext            = nullptr,
			.flags            = VULKAN_NO_FLAGS,
			.pInheritanceInfo = nullptr,
		};
		if(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS){
			evo::log::fatal("Failed to create Vulkan command buffer");
			return evo::resultError;
		}



		// Barrier to ensure that input buffer transfer is finished before compute shader reads from it
		auto buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_HOST_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = device_buffer,
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VULKAN_NO_FLAGS,
			0, nullptr,
			1, &buffer_barrier,
			0, nullptr
		);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		vkCmdBindDescriptorSets(
			command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set, 0, 0
		);

		vkCmdDispatch(command_buffer, NUM_BUFFER_ELEMS, 1, 1);

		// Barrier to ensure that shader writes are finished before buffer is read back from GPU
		buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = device_buffer,
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VULKAN_NO_FLAGS,
			0, nullptr,
			1, &buffer_barrier,
			0, nullptr
		);

		// Read back to host visible buffer
		const auto copy_region = VkBufferCopy(0, 0, BUFFER_SIZE);
		vkCmdCopyBuffer(command_buffer, device_buffer, host_buffer, 1, &copy_region);

		// Barrier to ensure that buffer copy is finished before host reading from it
		buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_HOST_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = host_buffer,
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_HOST_BIT,
			VULKAN_NO_FLAGS,
			0, nullptr,
			1, &buffer_barrier,
			0, nullptr
		);

		if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS){
			evo::log::fatal("Failed to end Vulkan command buffer");
			return evo::resultError;
		}

		// Submit compute work
		vkResetFences(device, 1, &fence);
		const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		const auto compute_submit_info = VkSubmitInfo{
			.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext                = nullptr,
			.waitSemaphoreCount   = 0,
			.pWaitSemaphores      = nullptr,
			.pWaitDstStageMask    = &wait_stage_mask,
			.commandBufferCount   = 1,
			.pCommandBuffers      = &command_buffer,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores    = nullptr,
		};

		if(vkQueueSubmit(queue, 1, &compute_submit_info, fence) != VK_SUCCESS){
			evo::log::fatal("Failed to submit Vulkan queue");
			return evo::resultError;
		}
		if(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS){
			evo::log::fatal("Failed to wait for Vulkan fences");
			return evo::resultError;
		}

		// Make device writes visible to the host
		void* mapped;
		vkMapMemory(device, host_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
		const auto mapped_range = VkMappedMemoryRange{
			.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			.pNext  = nullptr,
			.memory = host_memory,
			.offset = 0,
			.size   = VK_WHOLE_SIZE,
		};
		vkInvalidateMappedMemoryRanges(device, 1, &mapped_range);

		// Copy to output
		std::memcpy(compute_output.data(), mapped, BUFFER_SIZE);
		vkUnmapMemory(device, host_memory);


		///////////////////////////////////
		// done

		vkQueueWaitIdle(queue);


		for(uint32_t value : compute_input){
			evo::printMagenta("{:3}", value);
		}
		evo::println();


		for(uint32_t value : compute_output){
			evo::printCyan("{:3}", value);
		}
		evo::println();



		///////////////////////////////////
		// clean up

		vkDestroyFence(device, fence, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyShaderModule(device, shader_module, nullptr);
		vkDestroyPipelineCache(device, pipeline_cache, nullptr);
		vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

		vkDestroyBuffer(device, host_buffer, nullptr);
		vkFreeMemory(device, host_memory, nullptr);

		vkDestroyBuffer(device, device_buffer, nullptr);
		vkFreeMemory(device, device_memory, nullptr);

		vkDestroyCommandPool(device, command_pool, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);

		return evo::Result<>();
	}

	
}