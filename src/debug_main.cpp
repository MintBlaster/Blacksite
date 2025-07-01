#include <iostream>
#include "blacksite/core/Logger.h"

using namespace Blacksite;

int main() {
    std::cout << "DEBUG: Starting simple logger test..." << std::endl;
    
    try {
        std::cout << "DEBUG: About to call Logger::Instance()..." << std::endl;
        Logger& logger = Logger::Instance();
        
        std::cout << "DEBUG: Logger instance created, about to Initialize()..." << std::endl;
        logger.Initialize();
        
        std::cout << "DEBUG: Logger initialized successfully!" << std::endl;
        
        std::cout << "DEBUG: Testing basic log message..." << std::endl;
        BS_INFO(LogCategory::CORE, "Test message from debug main");
        
        std::cout << "DEBUG: Shutting down..." << std::endl;
        logger.Shutdown();
        
        std::cout << "DEBUG: Test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "DEBUG: Exception caught: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}