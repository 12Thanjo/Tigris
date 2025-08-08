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


	class Fence{
		public:
			Fence() = default;

			~Fence(){ if(this->isInitialized()){ this->deinit(); } }

			Fence(const Fence&) = delete;
			auto operator=(const Fence&) -> Fence& = delete;

			Fence(Fence&& rhs) : fence(std::exchange(rhs.fence, VK_NULL_HANDLE)), _device(rhs._device) {}
			auto operator=(Fence&& rhs) -> Fence& {
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
				return *this;
			}



			EVO_NODISCARD auto init(VkDevice device) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;

				const auto fence_create_info = VkFenceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.pNext = nullptr,
					.flags = utils::NO_FLAGS,
				};

				const VkResult result = vkCreateFence(this->_device, &fence_create_info, nullptr, &this->fence);
				if(utils::checkResult(result, "vkCreateFence").isError()){ return evo::resultError; }

				return evo::Result<>();
			}

			EVO_NODISCARD auto initAlreadySignaled(VkDevice device) -> evo::Result<> {
				evo::debugAssert(this->isInitialized() == false, "Already initialized");

				this->_device = device;

				const auto fence_create_info = VkFenceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.pNext = nullptr,
					.flags = VK_FENCE_CREATE_SIGNALED_BIT,
				};

				const VkResult result = vkCreateFence(this->_device, &fence_create_info, nullptr, &this->fence);
				if(utils::checkResult(result, "vkCreateFence").isError()){ return evo::resultError; }

				return evo::Result<>();
			}




			auto deinit() -> void {
				evo::debugAssert(this->isInitialized(), "Not initialized");

				vkDestroyFence(this->_device, this->fence, nullptr);
				this->fence = VK_NULL_HANDLE;
			}



			EVO_NODISCARD auto wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) -> evo::Result<> {
				const VkResult result = vkWaitForFences(this->_device, 1, &this->fence, VK_TRUE, timeout);
				if(utils::checkResult(result, "vkWaitForFences").isError()){ return evo::resultError; }
				
				return evo::Result<>();
			}

			EVO_NODISCARD auto waitAndReset(uint64_t timeout = std::numeric_limits<uint64_t>::max()) -> evo::Result<> {
				const VkResult result = vkWaitForFences(this->_device, 1, &this->fence, VK_TRUE, timeout);
				if(utils::checkResult(result, "vkWaitForFences").isError()){ return evo::resultError; }

				this->reset();
			
				return evo::Result<>();
			}


			// timeout is num of nanoseconds, may be longer (implementation defined)
			EVO_NODISCARD static auto waitAll(
				evo::ArrayProxy<Fence> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()
			) -> evo::Result<> {
				return multi_wait_impl<false>(fences, VK_TRUE, timeout);
			}

			// Wait for at least one
			// timeout is num of nanoseconds, may be longer (implementation defined)
			EVO_NODISCARD static auto waitOne(
				evo::ArrayProxy<Fence> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()
			) -> evo::Result<> {
				return multi_wait_impl<false>(fences, VK_FALSE, timeout);
			}



			EVO_NODISCARD static auto waitAllAndReset(
				evo::ArrayProxy<Fence> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()
			) -> evo::Result<> {
				return multi_wait_impl<true>(fences, VK_TRUE, timeout);
			}

			// Wait for at least one
			// timeout is num of nanoseconds, may be longer (implementation defined)
			EVO_NODISCARD static auto waitOneAndReset(
				evo::ArrayProxy<Fence> fences, uint64_t timeout = std::numeric_limits<uint64_t>::max()
			) -> evo::Result<> {
				return multi_wait_impl<true>(fences, VK_FALSE, timeout);
			}




			auto reset() -> void { vkResetFences(this->_device, 1, &this->fence); }

			static auto reset(evo::ArrayProxy<Fence> fences) -> void {
				debug_check_fences_compatable(fences);

				auto native_fences = evo::SmallVector<VkFence>();
				native_fences.reserve(fences.size());

				for(const Fence& fence : fences){
					native_fences.emplace_back(fence.native());
				}

				vkResetFences(fences[0]._device, uint32_t(native_fences.size()), native_fences.data());
			}




			EVO_NODISCARD auto isSignaled() const -> evo::Result<bool> {
				const VkResult result = vkGetFenceStatus(this->_device, this->fence);

				switch(result){
					case VK_SUCCESS:   return true;
					case VK_NOT_READY: return true;

					default: {
						std::ignore = utils::checkResult(result, "vkGetFenceStatus");
						return evo::resultError;
					} 
				}
			}



			EVO_NODISCARD auto isInitialized() const -> bool { return this->fence != VK_NULL_HANDLE; }
			EVO_NODISCARD auto native() const -> VkFence { return this->fence; }
			

		private:
			template<bool SHOULD_RESET>
			static auto multi_wait_impl(
				evo::ArrayProxy<Fence> fences, VkBool32 wait_all, uint64_t timeout
			) -> evo::Result<> {
				debug_check_fences_compatable(fences);

				auto native_fences = evo::SmallVector<VkFence>();
				native_fences.reserve(fences.size());

				for(const Fence& fence : fences){
					native_fences.emplace_back(fence.native());
				}

				const VkResult result = vkWaitForFences(
					fences[0]._device, uint32_t(native_fences.size()), native_fences.data(), wait_all, timeout
				);

				if(utils::checkResult(result, "vkWaitForFences").isError()){ return evo::resultError; }

				if constexpr(SHOULD_RESET){
					vkResetFences(fences[0]._device, uint32_t(native_fences.size()), native_fences.data());
				}
				
				return evo::Result<>();
			}



			static auto debug_check_fences_compatable(evo::ArrayProxy<Fence> fences) -> void {
				#if defined(_DEBUG)
					evo::debugAssert(fences.empty() == false, "Requires at least one fence");
					evo::debugAssert(fences.size() == 1, "For 1 fence, use the non-static method instead");
					evo::debugAssert(fences[0].isInitialized(), "All fences must be initialized");
					if(fences.size() > 1){
						for(size_t i = 1; i < fences.size(); i+=1){
							evo::debugAssert(
								fences[i]._device == fences[0]._device, "All fences must have the same device"
							);
							evo::debugAssert(fences[i].isInitialized(), "All fences must be initialized");
						}
					}
				#endif
			}



		private:
			VkFence fence = VK_NULL_HANDLE;
			VkDevice _device = VK_NULL_HANDLE;
	};

	
}