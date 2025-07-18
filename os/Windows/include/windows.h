////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Evo.h>

namespace os::windows{

	#if defined(EVO_PLATFORM_WINDOWS)
	

		auto setConsoleToUTF8Mode() -> void;

		EVO_NODISCARD auto isDebuggerPresent() -> bool;


	#endif


}