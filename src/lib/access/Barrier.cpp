#include "Barrier.h"

namespace hyrise { namespace access {
	bool Barrier::is_registered = QueryParser::registerPlanOperation<Barrier>("Barrier");
}}