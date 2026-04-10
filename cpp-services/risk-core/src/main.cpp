#include "trading/common/service_runtime.h"

#include <iostream>
#include <string_view>

namespace {

const trading::common::ServiceMetadata kServiceMetadata{
    "risk-core",
    "Real-time portfolio and exposure risk calculation core",
    "0.1.0",
};

}  // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string_view(argv[1]) == "--self-check") {
        return trading::common::RunSelfCheck(kServiceMetadata, std::cout, std::cerr);
    }

    std::cout << trading::common::BuildStartupBanner(kServiceMetadata) << '\n';
    std::cout << "Next steps: implement risk factor ingestion, rule evaluation and event output.\n";
    return 0;
}

