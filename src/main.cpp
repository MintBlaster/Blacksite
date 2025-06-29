#include "blacksite/blacksite.h"
#include <iostream>

int main()
{

    // --- Create engine instance ---
    Blacksite::Engine engine;

    // --- Initialize engine ---
    if (!engine.Initialize(1280, 720, "Blacksite engine test"))
    {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }

    // --- Run the engine ---
    return engine.Run();
}