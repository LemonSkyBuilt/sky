#pragma once

#include <ostream>
#include <string>

namespace trading::common {

struct ServiceMetadata {
    std::string name;
    std::string responsibility;
    std::string version;
};

std::string BuildStartupBanner(const ServiceMetadata& metadata);

int RunSelfCheck(const ServiceMetadata& metadata, std::ostream& out, std::ostream& err);

}  // namespace trading::common
