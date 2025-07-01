#include <iostream>
#include "blacksite/core/Logger.h"

using namespace Blacksite;

int main() {
    std::cout << "Starting logger-only test..." << std::endl;
    
    try {
        std::cout << "1. Creating logger instance..." << std::endl;
        Logger& logger = Logger::Instance();
        
        std::cout << "2. Initializing logger..." << std::endl;
        logger.Initialize();
        
        std::cout << "3. Testing basic logging..." << std::endl;
        BS_INFO(LogCategory::CORE, "Logger test message");
        BS_WARN(LogCategory::CORE, "This is a warning");
        BS_ERROR(LogCategory::CORE, "This is an error");
        
        std::cout << "4. Testing formatted logging..." << std::endl;
        BS_INFO_F(LogCategory::CORE, "Formatted message with number: %d", 42);
        BS_DEBUG_F(LogCategory::CORE, "Debug message with string: %s", "hello");
        
        std::cout << "5. Testing different categories..." << std::endl;
        BS_INFO(LogCategory::RENDERER, "Renderer message");
        BS_INFO(LogCategory::PHYSICS, "Physics message");
        BS_INFO(LogCategory::EDITOR, "Editor message");
        
        std::cout << "6. Checking log file creation..." << std::endl;
        const auto& history = logger.GetLogHistory();
        std::cout << "Log history contains " << history.size() << " entries" << std::endl;
        
        std::cout << "7. Shutting down..." << std::endl;
        logger.Shutdown();
        
        std::cout << "8. Logger test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}