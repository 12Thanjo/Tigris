////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigrisblob/main/LICENSE`for info.             //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/Engine.h"


#include "../include/utils.h"

#include <algorithm>


namespace vulkan{


	//////////////////////////////////////////////////////////////////////
	// vulkan helpers


	#if defined(_DEBUG)

			static VKAPI_ATTR auto VKAPI_CALL vk_debug_callback(
				VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
				VkDebugUtilsMessageTypeFlagsEXT message_type,
				const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
				[[maybe_unused]] void* user_data
			) -> VkBool32 {

				const std::string_view message_type_str = [&](){
					switch(message_type){
						case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:     return "GENERAL";
						case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:  return "VALIDATION";
						case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "PERFORMANCE";
					};

					evo::debugFatalBreak("Unknown message type");
				}();


				const std::string message = std::format(
					"{{VULKAN:{}}} {}", message_type_str, std::string_view(callback_data->pMessage)
				);
				
				switch(message_severity){
					break; case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
						evo::log::error(message);
						
						#if defined(_DEBUG)
							evo::breakpoint();
						#endif
					}
					break; case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: evo::log::warning(message);
					break; case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    evo::log::debug(message);
					break; case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: evo::log::trace(message);
					break; default: evo::debugFatalBreak("Debug callback recieved unknown Severity type");
				};

				return false;
			};


	#endif





	//////////////////////////////////////////////////////////////////////
	// engine

	static std::atomic<bool> engine_is_initialized = false;

	
	auto Engine::init() -> evo::Result<> {
		if(engine_is_initialized.exchange(true)){ evo::debugFatalBreak("Engine already exists"); }

		if(volkInitialize() != VK_SUCCESS){
			return evo::resultError;
		}


		if(this->create_instance().isError()){
			this->deinit_due_to_fail_to_initailize();
			return evo::resultError;
		}

		if(this->select_physical_device().isError()){
			this->deinit_due_to_fail_to_initailize();
			return evo::resultError;
		}

		if(this->create_device_and_queue().isError()){
			this->deinit_due_to_fail_to_initailize();
			return evo::resultError;
		}


		return evo::Result<>();
	}



	auto Engine::deinit() -> void {
		evo::debugAssert(this->isInitialized(), "Engine was not initialized");

		vkDestroyDevice(this->device, nullptr);

		#if defined(_DEBUG)
			vkDestroyDebugUtilsMessengerEXT(this->instance, this->debug_messenger, nullptr);
		#endif
		vkDestroyInstance(this->instance, nullptr);

		volkFinalize();


		engine_is_initialized = false;
	}



	auto Engine::isInitialized() const -> bool {
		return engine_is_initialized.load();
	}




	auto Engine::deinit_due_to_fail_to_initailize() -> void {
		if(this->device != VK_NULL_HANDLE){ vkDestroyDevice(this->device, nullptr); }

		#if defined(_DEBUG)	
			if(this->debug_messenger != VK_NULL_HANDLE){
				vkDestroyDebugUtilsMessengerEXT(this->instance, this->debug_messenger, nullptr);	
			}
		#endif
		if(this->instance != VK_NULL_HANDLE){ vkDestroyInstance(this->instance, nullptr); }

		engine_is_initialized = false;
	}




	auto Engine::create_instance() -> evo::Result<> {
		const auto app_info = VkApplicationInfo{
			.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext              = nullptr,
			.pApplicationName   = "Tigris",
			.applicationVersion = VK_MAKE_API_VERSION(0, 0, 5, 0),
			.pEngineName        = "Tigris Engine",
			.engineVersion      = VK_MAKE_API_VERSION(0, 0, 5, 0),
			.apiVersion         = VK_API_VERSION_1_1,
		};



		///////////////////////////////////
		// layers and extensions

		auto required_layers = evo::StaticVector<const char*, 1>();

		#if defined(_DEBUG)
			required_layers.emplace_back("VK_LAYER_KHRONOS_validation");
		#endif


		auto required_extensions = evo::StaticVector<const char*, 1>{};
		#if defined(_DEBUG)
			required_extensions.emplace_back("VK_EXT_debug_utils");
		#endif


		///////////////////////////////////
		// startup debug messenger

		#if defined(_DEBUG)
			const auto debug_messenger_create_info = VkDebugUtilsMessengerCreateInfoEXT{
				.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext           = nullptr,
				.flags           = utils::NO_FLAGS,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
									| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
				                  	| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				                  	| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
									| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
									| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = vk_debug_callback,
				.pUserData       = nullptr,
			};
		#endif


		const auto instance_create_info = VkInstanceCreateInfo{
			.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

			#if defined(_DEBUG)
				.pNext               = &debug_messenger_create_info,
			#else
				.pNext               = nullptr,
			#endif

			.flags                   = utils::NO_FLAGS,
			.pApplicationInfo        = &app_info,
			.enabledLayerCount       = uint32_t(required_layers.size()),
			.ppEnabledLayerNames     = required_layers.data(),
			.enabledExtensionCount   = uint32_t(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data(),
		};

		const VkResult create_instance_result = vkCreateInstance(&instance_create_info, nullptr, &this->instance);
		if(utils::checkResult(create_instance_result, "vkCreateInstance").isError()){ return evo::resultError; }


		#if defined(_DEBUG)
			vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				this->instance, "vkCreateDebugUtilsMessengerEXT"
			);

			if(vkCreateDebugUtilsMessengerEXT == nullptr){
				evo::log::error("vkCreateDebugUtilsMessengerEXT() not found");
				return evo::resultError;
			}

			const VkResult debug_utils_create_result = vkCreateDebugUtilsMessengerEXT(
				this->instance, &debug_messenger_create_info, nullptr, &this->debug_messenger
			);
			if(utils::checkResult(debug_utils_create_result, "vkCreateDebugUtilsMessengerEXT").isError()){
				return evo::resultError;
			}
		#endif


		volkLoadInstance(this->instance);

		return evo::Result<>();
	}



	auto Engine::select_physical_device() -> evo::Result<> {
		evo::SmallVector<VkPhysicalDevice> physical_devices = [&](){
			uint32_t num_physical_devices = 0;
			vkEnumeratePhysicalDevices(this->instance, &num_physical_devices, nullptr);

			auto physical_devices_list = evo::SmallVector<VkPhysicalDevice>(num_physical_devices);
			vkEnumeratePhysicalDevices(this->instance, &num_physical_devices, physical_devices_list.data());

			return physical_devices_list;
		}();

		if(physical_devices.empty()){
			evo::log::error("No supported physical devices (such as GPUs)");
			return evo::resultError;
		}

		evo::log::debug("Found {} physical devices", physical_devices.size());

		const evo::SmallVector<VkPhysicalDevice>::iterator end_of_physical_devices = std::remove_if(
			physical_devices.begin(),
			physical_devices.end(),
			[&](const VkPhysicalDevice& target_physical_device) -> bool {
				uint32_t num_queue_families;
				vkGetPhysicalDeviceQueueFamilyProperties(target_physical_device, &num_queue_families, nullptr);

				auto queue_family_props = evo::SmallVector<VkQueueFamilyProperties>(num_queue_families);
				vkGetPhysicalDeviceQueueFamilyProperties(
					target_physical_device, &num_queue_families, queue_family_props.data()
				);

				for(const VkQueueFamilyProperties& queue_family_prop : queue_family_props){
					if((queue_family_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0){ return false; }
				}

				return true;
			}
		);

		while(physical_devices.end() != end_of_physical_devices){
			physical_devices.pop_back();
		}

		if(physical_devices.empty()){
			evo::log::error("No supported physical devices (such as GPUs)");
			return evo::resultError;
		}


		const auto best_physical_device = std::max_element(physical_devices.begin(), physical_devices.end(), 
			[](const VkPhysicalDevice& lhs, const VkPhysicalDevice& rhs) -> bool {
				// return false if lhs is better
				// return true if rhs is better
				// return true if equal


				//////////////////
				// sort on type

				const auto get_device_type_score = [](const VkPhysicalDevice& target_physical_device) -> unsigned {
					auto props = VkPhysicalDeviceProperties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
					vkGetPhysicalDeviceProperties2(target_physical_device, &props);

					switch(props.properties.deviceType){
						break; case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return 4;
						break; case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return 3;
						break; case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 2;
						break; case VK_PHYSICAL_DEVICE_TYPE_CPU:            return 1;
						break; case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return 0;

					};

					return 0;
				};
				const unsigned lhs_device_type_score = get_device_type_score(lhs);
				const unsigned rhs_device_type_score = get_device_type_score(rhs);
				if(lhs_device_type_score != rhs_device_type_score){
					return lhs_device_type_score < rhs_device_type_score;
				}


				//////////////////
				// sort on memory size

				struct MemorySizes{
					VkDeviceSize local;
					VkDeviceSize shared;
				};

				const auto get_device_memory_sizes = [](const VkPhysicalDevice& target_physical_device) -> MemorySizes {
					auto memory_properties = VkPhysicalDeviceMemoryProperties2{
						VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
					};
					vkGetPhysicalDeviceMemoryProperties2(target_physical_device, &memory_properties);


					auto output = MemorySizes(0, 0);

					for(uint32_t i = 0; i < memory_properties.memoryProperties.memoryHeapCount; i++){
						if(memory_properties.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
							output.local += memory_properties.memoryProperties.memoryHeaps[i].size;
						}else{
							output.shared += memory_properties.memoryProperties.memoryHeaps[i].size;
						}
					}

					return output;
				};

				const MemorySizes lhs_memory_sizes = get_device_memory_sizes(lhs);
				const MemorySizes rhs_memory_sizes = get_device_memory_sizes(rhs);

				if(lhs_memory_sizes.local != rhs_memory_sizes.local){
					return lhs_memory_sizes.local < rhs_memory_sizes.local;
				}

				return lhs_memory_sizes.shared <= rhs_memory_sizes.shared;
			}
		);

		this->print_physical_device_properties(*best_physical_device);

		this->physical_device = *best_physical_device;

		return evo::Result<>();
	}



	auto Engine::create_device_and_queue() -> evo::Result<> {
		const auto queue_priorities = std::array<float, 1>{1.0f};

		this->queue_family_index = [&](){
			uint32_t num_queue_families;
			vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &num_queue_families, nullptr);

			auto queue_family_props = evo::SmallVector<VkQueueFamilyProperties>(num_queue_families);
			vkGetPhysicalDeviceQueueFamilyProperties(
				this->physical_device, &num_queue_families, queue_family_props.data()
			);

			for(size_t i = 0; const VkQueueFamilyProperties& queue_family_prop : queue_family_props){
				if((queue_family_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0){
					return uint32_t(i);
				}
			
				i += 1;
			}

			evo::debugFatalBreak(
				"Engine::select_physical_device() should have picked one that has a queue that supports compute"
			);
		}();

		const auto queue_create_info = VkDeviceQueueCreateInfo{
			.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext            = nullptr,
			.flags            = utils::NO_FLAGS,
			.queueFamilyIndex = this->queue_family_index,
			.queueCount       = uint32_t(queue_priorities.size()),
			.pQueuePriorities = queue_priorities.data(),
		};

		const auto device_create_info = VkDeviceCreateInfo{
			.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext                   = nullptr,
			.flags                   = utils::NO_FLAGS,
			.queueCreateInfoCount    = 1,
			.pQueueCreateInfos       = &queue_create_info,
			.enabledLayerCount       = 0,       // depricated
			.ppEnabledLayerNames     = nullptr, // depricated
			.enabledExtensionCount   = 0,
			.ppEnabledExtensionNames = nullptr,
			.pEnabledFeatures        = nullptr,
		};


		const VkResult result = vkCreateDevice(this->physical_device, &device_create_info, nullptr, &this->device);
		if(utils::checkResult(result, "vkCreateDevice").isError()){ return evo::resultError; }

		this->queue = Queue(this->device, this->queue_family_index, 0);

		return evo::Result<>();
	}




	auto Engine::print_physical_device_properties(VkPhysicalDevice target_physical_device) const -> void {
		auto properties = VkPhysicalDeviceProperties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		vkGetPhysicalDeviceProperties2(target_physical_device, &properties);

		auto features = VkPhysicalDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		vkGetPhysicalDeviceFeatures2(target_physical_device, &features);

		auto memory_properties = VkPhysicalDeviceMemoryProperties2{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
		};
		vkGetPhysicalDeviceMemoryProperties2(target_physical_device, &memory_properties);


		evo::log::info("Vulkan Device: {}", properties.properties.deviceName);

		switch(properties.properties.deviceType){
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
				evo::log::info("\tGPU Type:       Discrete");
			} break;

			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
				evo::log::info("\tGPU Type:       Integrated");
			} break;

			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
				evo::log::info("\tGPU Type:       CPU");
			} break;

			case VK_PHYSICAL_DEVICE_TYPE_CPU: {
				evo::log::info("\tGPU Type:       Virtual");
			} break;

			default: {
				evo::log::info("\tGPU Type:       Other / Unknown");
			} break;
		};


		evo::log::info(
			"\tAPI Version:    {}.{}.{}",
			VK_VERSION_MAJOR(properties.properties.apiVersion),
			VK_VERSION_MINOR(properties.properties.apiVersion),
			VK_VERSION_PATCH(properties.properties.apiVersion)
		);


		if(properties.properties.vendorID == 0x10de){
			// NVIDIA
			evo::log::info(
				"\tDriver Version: {}.{}.{}.{}",
				(properties.properties.driverVersion >> 22) & 0x3ff,
				(properties.properties.driverVersion >> 14) & 0x0ff,
				(properties.properties.driverVersion >> 6) & 0x0ff,
				properties.properties.driverVersion & 0x003f
			);

		#if defined(PH_PLATFORM_WINDOWS)
			}else if(properties.properties.vendorID == 0x8086){
				// Intel
				evo::log::info(
					"\tDriver Version: ??.??.{}.{} (unformatted: 0x{:x})",
					(properties.properties.driverVersion >> 14) & 0xff,
					properties.properties.driverVersion & 0x3fff,
					properties.properties.driverVersion
				);
				evo::log::trace("\t(Intel Driver version formatting is really weird. "
								"The first values cannot be obtained through Vulkan)");

		#endif

		}else{
			// Default
			evo::log::info(
				"\tDriver Version: {}.{}.{} (unformatted: 0x{:x})",
				VK_VERSION_MAJOR(properties.properties.driverVersion),
				VK_VERSION_MINOR(properties.properties.driverVersion),
				VK_VERSION_PATCH(properties.properties.driverVersion),
				properties.properties.apiVersion
			);
		}


		const std::string_view vendor_id_name = [&]() -> std::string_view {
			switch(properties.properties.vendorID){
				case 0x10de: return "NVIDIA";
				case 0x8086: return "Intel";
				case 0x1002: return "AMD";
				case 0x13B5: return "ARM";
				case 0x5143: return "Qualcomm";
				default:     return "Unknown";
			};
		}();


		evo::log::info("\tVendor ID:      {:x} ({})", properties.properties.vendorID, vendor_id_name);
		evo::log::info("\tDevice ID:      {:x}", properties.properties.deviceID);


		// memory information
		for(uint32_t i = 0; i < memory_properties.memoryProperties.memoryHeapCount; i++){
			float memory_size_gib = float(memory_properties.memoryProperties.memoryHeaps[i].size)
				 / 1024.0f / 1024.0f / 1024.0f;

			if(memory_properties.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
				evo::log::info("\tGPU memory heap [{}] (Local):  {} GiB", i, memory_size_gib);
			}else{
				evo::log::info("\tGPU memory heap [{}] (Shared): {} GiB", i, memory_size_gib);
			}
		}


	}





	
}