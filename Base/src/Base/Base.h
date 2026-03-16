#pragma once

#include "Base/Basetypes.h"

#define KILO_BYTE 1024
#define MEGA_BYTE (KILO_BYTE * KILO_BYTE)
#define GIGA_BYTE (MEGA_BYTE * KILO_BYTE)

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(*arr))

#ifdef PH_PLATFORM_WINDOWS
#include <Windows.h>

	#define PH_BREAK() {DebugBreak();}
	#define PH_ASSERT_BREAK(x, message) {if(!(x)) {DebugBreak();}}

	#ifdef PH_DEBUG
		#define PH_DEBUG_BREAK() DebugBreak();
		#define PH_DEBUG_MESSAGE(message) DebugBreak();
		#define	PH_DEBUG_ASSERT(x, message) {if(!(x)) {DebugBreak();}}
	#else
		#define PH_DEBUG_BREAK()
		#define PH_DEBUG_MESSAGE(message)
		#define	PH_DEBUG_ASSERT(x, message)
	#endif // DEBUG
#endif // DEBUG

