// Copyright dSPACE GmbH. All rights reserved.

#include "BenchmarkHelper.h"

namespace DsVeosCoSim {

void OnLogCallback(Severity severity, std::string_view message) {
    switch (severity) {
        case Severity::Error:
            printf("ERROR %s\n", message.data());
            break;
        case Severity::Warning:
            printf("WARN  %s\n", message.data());
            break;
        case Severity::Info:
            printf("INFO  %s\n", message.data());
            break;
        case Severity::Trace:
            printf("TRACE %s\n", message.data());
            break;
        default:  // NOLINT(clang-diagnostic-covered-switch-default)
            break;
    }
}

}  // namespace DsVeosCoSim
