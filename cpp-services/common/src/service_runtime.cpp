#include "trading/common/service_runtime.h"

#include <sstream>

namespace trading::common {
namespace {

bool HasEmptyField(const ServiceMetadata& metadata) {
    return metadata.name.empty() || metadata.responsibility.empty() || metadata.version.empty();
}

}  // namespace

std::string BuildStartupBanner(const ServiceMetadata& metadata) {
    std::ostringstream banner;
    banner << metadata.name << " [" << metadata.version << "]\n"
           << "Responsibility: " << metadata.responsibility << '\n'
           << "Status: skeleton bootstrapped";
    return banner.str();
}

int RunSelfCheck(const ServiceMetadata& metadata, std::ostream& out, std::ostream& err) {
    if (HasEmptyField(metadata)) {
        err << "Self-check failed: service metadata must not be empty.\n";
        return 1;
    }

    const std::string banner = BuildStartupBanner(metadata);
    if (banner.find(metadata.name) == std::string::npos ||
        banner.find(metadata.responsibility) == std::string::npos ||
        banner.find(metadata.version) == std::string::npos) {
        err << "Self-check failed: startup banner is missing required fields.\n";
        return 1;
    }

    out << "Self-check passed for " << metadata.name << ".\n";
    return 0;
}

}  // namespace trading::common
