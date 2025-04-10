#pragma once

#include "ErrorManager.h"

#if _DEBUG
#define ASSERT(x) if (!(x)) { ErrorManager::Get()->ReportError(ErrorSeverity::Severe, __FUNCTION__, "Assert", __LINE__, #x); }
#else
#define ASSERT(x);
#endif // if _DEBUG