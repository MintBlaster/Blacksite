#include <iostream>
#include "blacksite/core/Logger.h"

using namespace Blacksite;

int main() {
    std::cout << "Testing logger..." << std::endl;

    try {
        // Test singleton creation
        std::cout << "Creating logger instance..." << std::endl;
        Logger& logger = Logger::Instance();

        // Test basic logging without Initialize() first
        std::cout << "Testing direct logging..." << std::endl;
        logger.Info(LogCategory::CORE, "Test message 1");
        logger.Warn(LogCategory::CORE, "Test warning");
        logger.Error(LogCategory::CORE, "Test error");

        // Test macros
        std::cout << "Testing macros..." << std::endl;
        BS_INFO(LogCategory::CORE, "Macro test message");

        std::cout << "Logger test completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Logger test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Logger test failed with unknown exception" << std::endl;
        return 1;
    }

    return 0;
}
