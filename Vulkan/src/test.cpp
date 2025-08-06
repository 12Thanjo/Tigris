////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/test.h"

#include "../include/Fence.h"
#include "../include/Buffer.h"
#include "../include/CommandPool.h"
#include "../include/Engine.h"

#include <filesystem>


namespace vulkan{


	EVO_NODISCARD static auto create_shader_module(VkDevice device, const std::filesystem::path& path)
	-> evo::Result<VkShaderModule> {
		const evo::Result<std::vector<evo::byte>> file = evo::fs::readBinaryFile(path.string());
		if(file.isError()){
			evo::log::error("Failed to load shader at path: \"{}\"", path.string());
			return evo::resultError;
		}

		if(file.value().size() % 4 != 0){
			evo::log::error("File \"{}\" is not a valid shader file", path.string());
			return evo::resultError;
		}

		if(reinterpret_cast<const uint32_t*>(file.value().data())[0] != 0x07230203){
			evo::log::error("File \"{}\" is not a valid shader file", path.string());
			return evo::resultError;
		}

		const auto create_info = VkShaderModuleCreateInfo{
			.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext    = nullptr,
			.flags    = utils::NO_FLAGS,
			.codeSize = file.value().size(),
			.pCode    = reinterpret_cast<const uint32_t*>(file.value().data()),
		};

		VkShaderModule output;
		{
			const VkResult result = vkCreateShaderModule(device, &create_info, nullptr, &output);
			if(utils::checkResult(result, "vkCreateShaderModule").isError()){ return evo::resultError; }
		}

		return output;
	}




	auto test() -> evo::Result<> {
		evo::log::debug("Vulkan Test");


		auto engine = Engine();
		EVO_DEFER([&](){ engine.deinit(); });

		if(engine.init().isError()){ return evo::resultError; }



		///////////////////////////////////
		// create command pool


		auto command_pool = CommandPool();
		if(command_pool.init(engine.device, engine.queue_family_index).isError()){ return evo::resultError; }
		EVO_DEFER([&](){ command_pool.deinit(); });


		///////////////////////////////////
		// prepare storage buffers

		static constexpr size_t NUM_BUFFER_ELEMS = 60;

		auto compute_input = evo::SmallVector<uint32_t>(NUM_BUFFER_ELEMS);
		auto compute_output = evo::SmallVector<uint32_t>(NUM_BUFFER_ELEMS);

		for(size_t i = 0; i < NUM_BUFFER_ELEMS; i+=1){
			compute_input[i] = uint32_t(i);
		}

		static constexpr VkDeviceSize BUFFER_SIZE = NUM_BUFFER_ELEMS * sizeof(uint32_t);


		auto host_buffer = Buffer();
		EVO_DEFER([&](){ host_buffer.deinit(); });

		auto device_buffer = Buffer();
		EVO_DEFER([&](){ device_buffer.deinit(); });


		///////////////////////////////////
		// copy input data to VRAM iusing a staging buffer
		{
			if(host_buffer.init(
				engine.device,
				engine.physical_device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				BUFFER_SIZE
			).isError()){ return evo::resultError; }

			if(host_buffer.writeData(static_cast<evo::ArrayProxy<uint32_t>>(compute_input)).isError()){
				return evo::resultError;
			}
			host_buffer.flushWrite();


			if(device_buffer.init(
				engine.device,
				engine.physical_device,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
					| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
					| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				BUFFER_SIZE
			).isError()){ return evo::resultError; }


			//////////////////
			// copy to staging buffer

			{
				evo::Result<evo::SmallVector<CommandBuffer>> allocated_command_buffers = 
					command_pool.allocateCommandBuffers(1);
				if(allocated_command_buffers.isError()){ return evo::resultError; }

				CommandBuffer copy_cmd = std::move(allocated_command_buffers.value()[0]);


				if(copy_cmd.begin().isError()){ return evo::resultError; }
				copy_cmd.copyBuffer(host_buffer, device_buffer);
				if(copy_cmd.end().isError()){ return evo::resultError; }


				auto fence = Fence();
				if(fence.init(engine.device).isError()){ return evo::resultError; }


				//////////////////
				// Submit to the queue

				const auto submit_info = Queue::SubmitInfo(nullptr, nullptr, copy_cmd, nullptr);
				if(engine.queue.submit(submit_info, fence).isError()){ return evo::resultError; }

				if(fence.wait().isError()){ return evo::resultError; }

				fence.deinit();

				copy_cmd.deinit();
			}
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


		const auto pool_sizes = std::to_array<VkDescriptorPoolSize>({
			VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
		});
		const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext         = nullptr,
			.flags         = utils::NO_FLAGS,
			.maxSets       = 1,
			.poolSizeCount = uint32_t(pool_sizes.size()),
			.pPoolSizes    = pool_sizes.data(),
		};
		
		{
			const VkResult result =
				vkCreateDescriptorPool(engine.device, &descriptor_pool_info, nullptr, &descriptor_pool);
			if(utils::checkResult(result, "vkCreateDescriptorPool").isError()){ return evo::resultError; }
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
			.flags        = utils::NO_FLAGS,
			.bindingCount = uint32_t(set_layout_bindings.size()),
			.pBindings    = set_layout_bindings.data(),
		};
		
		{
			const VkResult result =
				vkCreateDescriptorSetLayout(engine.device, &descriptor_layout, nullptr, &descriptor_set_layout);
			if(utils::checkResult(result, "vkCreateDescriptorSetLayout").isError()){ return evo::resultError; }
		}


		const auto pipeline_create_info = VkPipelineLayoutCreateInfo{
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext                  = nullptr,
			.flags                  = utils::NO_FLAGS,
			.setLayoutCount         = 1,
			.pSetLayouts            = &descriptor_set_layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges    = nullptr,
		};
		
		{
			const VkResult result =
				vkCreatePipelineLayout(engine.device, &pipeline_create_info, nullptr, &pipeline_layout);
			if(utils::checkResult(result, "vkCreatePipelineLayout").isError()){ return evo::resultError; }
		}


		const auto alloc_info = VkDescriptorSetAllocateInfo{
			.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext              = nullptr,
			.descriptorPool     = descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts        = &descriptor_set_layout,
		};
		
		{
			const VkResult result = vkAllocateDescriptorSets(engine.device, &alloc_info, &descriptor_set);
			if(utils::checkResult(result, "vkAllocateDescriptorSets").isError()){ return evo::resultError; }
		}


		const auto buffer_descriptor = VkDescriptorBufferInfo(device_buffer.native(), 0, VK_WHOLE_SIZE);
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
			engine.device,
			uint32_t(compute_write_descriptor_sets.size()),
			compute_write_descriptor_sets.data(),
			0,
			nullptr
		);



		const auto pipline_cache_create_info = VkPipelineCacheCreateInfo{
			.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext           = nullptr,
			.flags           = utils::NO_FLAGS,
			.initialDataSize = 0,
			.pInitialData    = nullptr,
		};
		{
			const VkResult result = 
				vkCreatePipelineCache(engine.device, &pipline_cache_create_info, nullptr, &pipeline_cache);
			if(utils::checkResult(result, "vkCreatePipelineCache").isError()){ return evo::resultError; }
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
			create_shader_module(engine.device, "../Vulkan/shaders/compute.comp.spv");
		if(shader_module_res.isError()){ return evo::resultError; }

		shader_module = shader_module_res.value();


		//////////////////
		// create pipeline

		const auto compute_pipeline_create_info = VkComputePipelineCreateInfo{
			.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext              = nullptr,
			.flags              = utils::NO_FLAGS,
			.stage              = VkPipelineShaderStageCreateInfo{
				.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext               = nullptr,
				.flags               = utils::NO_FLAGS,
				.stage               = VK_SHADER_STAGE_COMPUTE_BIT,
				.module              = shader_module,
				.pName               = "main",
				.pSpecializationInfo = &specialization_info,
			},
			.layout             = pipeline_layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex  = 0,
		};

		{
			const VkResult result = vkCreateComputePipelines(
				engine.device, pipeline_cache, 1, &compute_pipeline_create_info, nullptr, &pipeline
			);
			if(utils::checkResult(result, "vkCreateComputePipelines").isError()){ return evo::resultError; }
		}


		evo::Result<evo::SmallVector<CommandBuffer>> allocated_command_buffers = 
			command_pool.allocateCommandBuffers(1);
		if(allocated_command_buffers.isError()){ return evo::resultError; }

		CommandBuffer command_buffer = std::move(allocated_command_buffers.value()[0]);




		// Fence for compute CB sync
		auto fence = Fence();
		if(fence.init(engine.device, VK_FENCE_CREATE_SIGNALED_BIT).isError()){ return evo::resultError; }


		///////////////////////////////////
		// create command buffer for compute work submission


		command_buffer.begin();



		// Barrier to ensure that input buffer transfer is finished before compute shader reads from it
		auto buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_HOST_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = device_buffer.native(),
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		command_buffer.pipelineBarrier(
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			utils::NO_FLAGS,
			nullptr,
			buffer_barrier,
			nullptr
		);

		command_buffer.bindPipeline(pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);

		command_buffer.bindDescriptorSets(
			VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline_layout,
			0,
			descriptor_set,
			nullptr
		);


		command_buffer.dispatch(NUM_BUFFER_ELEMS);

		buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = device_buffer.native(),
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		command_buffer.pipelineBarrier(
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			utils::NO_FLAGS,
			nullptr,
			buffer_barrier,
			nullptr
		);


		// Read back to host visible buffer
		command_buffer.copyBuffer(device_buffer, host_buffer);


		// Barrier to ensure that buffer copy is finished before host reading from it
		buffer_barrier = VkBufferMemoryBarrier{
			.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_HOST_READ_BIT,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer              = host_buffer.native(),
			.offset              = 0,
			.size                = VK_WHOLE_SIZE,
		};
		command_buffer.pipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_HOST_BIT,
			utils::NO_FLAGS,
			nullptr,
			buffer_barrier,
			nullptr
		);
		if(command_buffer.end().isError()){ return evo::resultError; }


		// Submit compute work
		fence.reset();

		const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		const auto compute_submit_info = Queue::SubmitInfo(nullptr, wait_stage_mask, command_buffer, nullptr);
		if(engine.queue.submit(compute_submit_info, fence).isError()){ return evo::resultError; }

		if(fence.wait().isError()){ return evo::resultError; }

		// Make device writes visible to the host
		compute_output = host_buffer.extractData<uint32_t>();


		///////////////////////////////////
		// done

		engine.queue.waitIdle();


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

		command_buffer.deinit();
		fence.deinit();
		vkDestroyPipeline(engine.device, pipeline, nullptr);
		vkDestroyShaderModule(engine.device, shader_module, nullptr);
		vkDestroyPipelineCache(engine.device, pipeline_cache, nullptr);
		vkDestroyPipelineLayout(engine.device, pipeline_layout, nullptr);
		vkDestroyDescriptorSetLayout(engine.device, descriptor_set_layout, nullptr);
		vkDestroyDescriptorPool(engine.device, descriptor_pool, nullptr);

		return evo::Result<>();
	}

	
}