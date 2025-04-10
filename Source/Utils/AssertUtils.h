#pragma once

#include "ErrorManager.h"

#if _DEBUG
#define ASSERT(x) if (!(x)) { ErrorManager::Get()->ReportError(ErrorSeverity::Severe, "_##func", "", 0, #x); }
#else
#define ASSERT(x);
#endif // if _DEBUG