#pragma once
#include <stdio.h>
#include "Base/Datastructures/String.h"

namespace PH::Base {

	#define CONSOLE_WRITE(name) void name(const PH::Base::SubString& str)
	typedef CONSOLE_WRITE(logfunc);

	extern logfunc* base_log;
	inline CONSOLE_WRITE(baselog) {
		if (base_log) {
			base_log(str);
		}
	}

	template<logfunc log>
	class LogStream {
	public:
		LogStream& operator<<(const SubString& str) {
			log(str);
			return *this;
		}

		LogStream& operator<<(const char* str) {
			log(str);
			return *this;
		}

		LogStream& operator<<(uint32 integer) {
			char buffer[16];
			sprintf_s<16>(buffer, "%u", integer);
			log(buffer);
			return *this;
		}

		LogStream& operator<<(uint64 integer) {
			char buffer[32];
			sprintf_s<32>(buffer, "%llu", integer);
			log(buffer);
			return *this;
		}

		LogStream& operator<<(int32 integer) {
			char buffer[16];
			sprintf_s<16>(buffer, "%i", integer);
			log(buffer);
			return *this;
		}

		LogStream& operator<<(real32 value) {
			char buffer[16];
			sprintf_s<16>(buffer, "%f", value);
			log(buffer);
			return *this;
		}

	private:
		SubString streamformat;
	};

	extern LogStream<baselog> INFO;
	extern LogStream<baselog> WARN;
	extern LogStream<baselog> ERR;
}
