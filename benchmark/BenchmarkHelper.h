// Copyright dSPACE GmbH. All rights reserved.

#pragma once

#include "CoSimTypes.h"

#include <stdexcept>  // IWYU pragma: keep

namespace DsVeosCoSim {

void OnLogCallback(Severity severity, std::string_view message);

#define ASSERT_OK(actual)                                \
    do {                                                 \
        if ((actual) != Result::Ok) {                    \
            throw std::runtime_error("Invalid result."); \
        }                                                \
    } while (0)

}  // namespace DsVeosCoSim
