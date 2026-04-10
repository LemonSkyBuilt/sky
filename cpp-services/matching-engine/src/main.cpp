#include "trading/common/service_runtime.h"

#include <iostream>
#include <string_view>

namespace {

const trading::common::ServiceMetadata kServiceMetadata{
    "matching-engine",
    "Low-latency order book and simulated matching core",
    "0.1.0",
};

}  // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string_view(argv[1]) == "--self-check") {
        return trading::common::RunSelfCheck(kServiceMetadata, std::cout, std::cerr);
    }

    std::cout << trading::common::BuildStartupBanner(kServiceMetadata) << '\n';
    std::cout << "Next steps: implement order intake, order book state and trade event emission.\n";
    return 0;
}

