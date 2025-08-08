////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/test.h"


#include "../include/Vulkan.h"


#include <filesystem>


namespace vulkan{




	auto test() -> evo::Result<> {
		evo::log::debug("Vulkan Test");


		auto engine = Engine();

		if(engine.init().isError()){ return evo::resultError; }



		auto fence = Fence();
		if(fence.init(engine.device).isError()){ return evo::resultError; }


		///////////////////////////////////
		// create command pool / command buffer

		auto command_pool = CommandPool();
		if(command_pool.init(engine.device, engine.queue_family_index).isError()){ return evo::resultError; }


		evo::Result<evo::SmallVector<CommandBuffer>> allocated_command_buffers = 
			command_pool.allocateCommandBuffers(1);
		if(allocated_command_buffers.isError()){ return evo::resultError; }

		CommandBuffer command_buffer = std::move(allocated_command_buffers.value()[0]);


		///////////////////////////////////
		// prepare storage buffers

		static constexpr size_t NUM_BUFFER_ELEMS = 60;

		auto compute_input = evo::SmallVector<uint32_t>(NUM_BUFFER_ELEMS);

		for(size_t i = 0; i < NUM_BUFFER_ELEMS; i+=1){
			compute_input[i] = uint32_t(i);
		}

		static constexpr VkDeviceSize BUFFER_SIZE = NUM_BUFFER_ELEMS * sizeof(uint32_t);


		auto host_buffer = Buffer();

		auto device_buffer = Buffer();




		///////////////////////////////////
		// copy input data to VRAM using a staging buffer

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

		if(command_buffer.begin().isError()){ return evo::resultError; }
		command_buffer.copyBuffer(host_buffer, device_buffer);
		if(command_buffer.end().isError()){ return evo::resultError; }



		//////////////////
		// Submit to the queue

		const auto submit_info = Queue::SubmitInfo(nullptr, nullptr, command_buffer, nullptr);
		if(engine.queue.submit(submit_info, fence).isError()){ return evo::resultError; }



		///////////////////////////////////
		// prepare compute pipeline

		auto descriptor_pool = DescriptorPool();
		if(descriptor_pool.init(
			engine.device, 1, VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
		).isError()){
			return evo::resultError;
		}


		const auto descriptor_set_layout_bindings = std::to_array<VkDescriptorSetLayoutBinding>({
			VkDescriptorSetLayoutBinding{
				.binding            = 0,
				.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount    = 1,
				.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
				.pImmutableSamplers = nullptr,
			},
		});

		auto descriptor_set_layout = DescriptorSetLayout();
		if(descriptor_set_layout.init(engine.device, descriptor_set_layout_bindings).isError()){
			return evo::resultError;
		}



		evo::Result<evo::SmallVector<VkDescriptorSet>> descriptor_sets = descriptor_pool.allocateDescriptorSets(
			descriptor_set_layout
		);
		if(descriptor_sets.isError()){ return evo::resultError; }

		VkDescriptorSet descriptor_set = std::move(descriptor_sets.value()[0]);


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


		//////////////////
		// create pipeline

		auto shader_module = ShaderModule();
		if(shader_module.initFromFile(engine.device, "../Vulkan/shaders/compute.comp.spv").isError()){
			return evo::resultError;
		}

		struct SpecializationData{
			uint32_t num_buffer_elems;
		};
		const auto specialization_data = SpecializationData(NUM_BUFFER_ELEMS);
		const auto specialization_map_entry = VkSpecializationMapEntry(0, 0, sizeof(uint32_t));
		const auto specialization_info = VkSpecializationInfo{
			.mapEntryCount = 1,
			.pMapEntries   = &specialization_map_entry,
			.dataSize      = sizeof(SpecializationData),
			.pData         = &specialization_data,
		};



		auto pipeline_layout = PipelineLayout();
		if(pipeline_layout.init(engine.device, descriptor_set_layout, nullptr).isError()){
			return evo::resultError;
		}

		auto pipeline_cache = PipelineCache();
		if(pipeline_cache.init(engine.device).isError()){ return evo::resultError; }


		auto pipeline = Pipeline();
		if(pipeline.initCompute(
			engine.device,
			shader_module.createPipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, specialization_info),
			pipeline_layout,
			pipeline_cache
		).isError()){ return evo::resultError; }




		///////////////////////////////////
		// create command buffer for compute work submission

		// wait for old work to complete
		if(fence.waitAndReset().isError()){ return evo::resultError; }


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

		const auto compute_submit_info = Queue::SubmitInfo(
			nullptr, VK_PIPELINE_STAGE_TRANSFER_BIT, command_buffer, nullptr
		);
		if(engine.queue.submit(compute_submit_info, fence).isError()){ return evo::resultError; }

		if(fence.waitAndReset().isError()){ return evo::resultError; }



		///////////////////////////////////
		// done

		const evo::SmallVector<uint32_t> compute_output =  host_buffer.extractData<uint32_t>();

		engine.queue.waitIdle();

		for(uint32_t value : compute_input){
			evo::printMagenta("{:3}", value);
		}
		evo::println();


		for(uint32_t value : compute_output){
			evo::printCyan("{:3}", value);
		}
		evo::println();


		return evo::Result<>();
	}

	
}