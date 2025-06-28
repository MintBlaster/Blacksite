#include "blacksite/core/window.h"
#include <iostream>

int main() {
    Blacksite::Window window;

    // Create 1280x720 window
    if (!window.Initialize(1280, 720, "Blacksite Test")) {
        return -1;
    }

    // Keep window open until user closes it
    while (!window.ShouldClose()) {
        // Clear screen to black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        window.SwapBuffers();  // Show frame
        window.PollEvents();   // Handle input
    }

    return 0;
}