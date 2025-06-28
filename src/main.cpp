#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/version.h>

// Try different bullet include paths
#ifdef __has_include
    #if __has_include(<btBulletDynamicsCommon.h>)
        #include <btBulletDynamicsCommon.h>
    #elif __has_include(<bullet/btBulletDynamicsCommon.h>)
        #include <bullet/btBulletDynamicsCommon.h>
    #elif __has_include(<BulletDynamics/btBulletDynamicsCommon.h>)
        #include <BulletDynamics/btBulletDynamicsCommon.h>
    #else
        #warning "Bullet Physics header not found, skipping physics test"
        #define NO_BULLET
    #endif
#else
    #include <bullet/btBulletDynamicsCommon.h>
#endif

#include <assimp/Importer.hpp>

void printLibraryVersions() {
    std::cout << "=== Blacksite Engine - Library Test ===" << std::endl;
    
    // GLFW version
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    std::cout << "GLFW Version: " << major << "." << minor << "." << rev << std::endl;
    
    // OpenGL version (after context creation)
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
    
    // GLM version
    std::cout << "GLM Version: " << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << std::endl;
    
    // Bullet Physics test
#ifndef NO_BULLET
    btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
    std::cout << "Bullet Physics: Initialized successfully" << std::endl;
    delete collisionConfig;
#else
    std::cout << "Bullet Physics: Skipped (header not found)" << std::endl;
#endif
    
    // Assimp test
    Assimp::Importer importer;
    std::cout << "Assimp: Version " << aiGetVersionRevision() << std::endl;
    
    std::cout << "All libraries loaded successfully!" << std::endl;
    std::cout << "=================================" << std::endl;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Blacksite Engine - Library Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Print library information
    printLibraryVersions();
    
    // Basic render loop test
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    int frameCount = 0;
    while (!glfwWindowShouldClose(window) && frameCount < 60) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        frameCount++;
    }
    
    std::cout << "Rendered " << frameCount << " frames successfully!" << std::endl;
    std::cout << "Press any key to close..." << std::endl;
    std::cin.get();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}