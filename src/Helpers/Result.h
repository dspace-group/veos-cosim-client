// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "Logger.h"  // IWYU pragma: keep

namespace DsVeosCoSim {

#define CheckResultWithMessage(result, message) \
    do {                                        \
        if (!(result)) [[unlikely]] {           \
            LogTrace(message);                  \
            return false;                       \
        }                                       \
    } while (0)

#define CheckResult(result)           \
    do {                              \
        if (!(result)) [[unlikely]] { \
            return false;             \
        }                             \
    } while (0)

}  // namespace DsVeosCoSim
