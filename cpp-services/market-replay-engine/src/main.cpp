#include "trading/common/service_runtime.h"

#include <iostream>
#include <string_view>

namespace {

const trading::common::ServiceMetadata kServiceMetadata{
    "market-replay-engine",
    "Historical market data replay driver for backtesting workflows",
    "0.1.0",
};

}  // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string_view(argv[1]) == "--self-check") {
        return trading::common::RunSelfCheck(kServiceMetadata, std::cout, std::cerr);
    }

    std::cout << trading::common::BuildStartupBanner(kServiceMetadata) << '\n';
    std::cout << "Next steps: implement playback scheduling, speed control and downstream publishing.\n";
    return 0;
}

