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


	class ShaderModule{
		public:
			ShaderModule() = default;

			~ShaderModule(){ if(this->isInitialized()){ this->deinit(); } }

			ShaderModule(const ShaderModule&) = delete;
			auto operator=(const ShaderModule&) -> ShaderModule& = delete;

			ShaderModule(ShaderModule&& rhs) : 
				shader_module(std::exchange(rhs.shader_module, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(ShaderModule&& rhs) -> ShaderModule& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(VkDevice device, evo::ArrayProxy<uint32_t> byte_code) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;

				const auto create_info = VkShaderModuleCreateInfo{
					.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
					.pNext    = nullptr,
					.flags    = utils::NO_FLAGS,
					.codeSize = byte_code.size(),
					.pCode    = byte_code.data(),
				};

				const VkResult result = vkCreateShaderModule(
					this->_device, &create_info, nullptr, &this->shader_module
				);
				return utils::checkResult(result, "vkCreateShaderModule");
			}


			EVO_NODISCARD auto initFromFile(VkDevice device, const std::filesystem::path& path) -> evo::Result<> {
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


				const auto byte_code = evo::ArrayProxy<uint32_t>(
					reinterpret_cast<const uint32_t*>(file.value().data()), file.value().size()
				);

				return this->init(device, byte_code);
			}




			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyShaderModule(this->_device, this->shader_module, nullptr);
				this->shader_module = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto createPipelineShaderStageCreateInfo(
				VkShaderStageFlagBits stage, 
				const VkSpecializationInfo& specialization_info,
				const char* entry_name = "main"
			) const -> VkPipelineShaderStageCreateInfo {
				return VkPipelineShaderStageCreateInfo{
					.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext               = nullptr,
					.flags               = utils::NO_FLAGS,
					.stage               = stage,
					.module              = this->shader_module,
					.pName               = entry_name,
					.pSpecializationInfo = &specialization_info,
				};
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->shader_module != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkShaderModule { return this->shader_module; }


		private:
			VkShaderModule shader_module = VK_NULL_HANDLE;
			VkDevice _device;
	};

	
}