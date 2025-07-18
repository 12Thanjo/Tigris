////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////


#include "../include/windows.h"


#if defined(EVO_PLATFORM_WINDOWS)
	#if !defined(WIN32_LEAN_AND_MEAN)
		#define WIN32_LEAN_AND_MEAN
	#endif

	#if !defined(NOCOMM)
		#define NOCOMM
	#endif

	#if !defined(NOMINMAX)
		#define NOMINMAX
	#endif

	#include <Windows.h>
#endif


namespace os::windows{


	#if defined(EVO_PLATFORM_WINDOWS)
	

		auto setConsoleToUTF8Mode() -> void {
			::SetConsoleOutputCP(CP_UTF8);
		}


		auto isDebuggerPresent() -> bool {
			return ::IsDebuggerPresent();
		}
		

	#endif

	
}
