#include "Log.h"

namespace PH::Base {

	logfunc* base_log;

	LogStream<baselog> INFO;
	LogStream<baselog> WARN;
	LogStream<baselog> ERR;

}