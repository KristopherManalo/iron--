#include <iostream>
#include "Application.h"

void Application::run() {
	std::cout << "Hello world\n";
	renderer.initialize(PROGRAM_NAME);
	mainLoop();

    cleanup();
}

void Application::mainLoop() {
    std::cout << "Press spacebar to quit\n" << std::endl;
    float speed = 10.0f;
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        glfwPollEvents();
#define getkeypress(key) (glfwGetKey(renderer.getWindow(), key) == GLFW_PRESS)
        if (getkeypress(GLFW_KEY_SPACE)) break;
        if (getkeypress(GLFW_KEY_W)) {
            std::cout << "HOLDING W :3" << std::endl;
        }
        if (getkeypress(GLFW_KEY_A)) {
            std::cout << "HOLDING A :3" << std::endl;
        }
        if (getkeypress(GLFW_KEY_S)) {
            std::cout << "HOLDING S :3" << std::endl;
        }
        if (getkeypress(GLFW_KEY_D)) {
            std::cout << "HOLDING D :3" << std::endl;
        }

		if (renderer.framebufferResized) {
			renderer.framebufferResized = false;
			renderer.recreateSwapChain();
            std::cout << "WAS RESIZED :3" << std::endl;
		}
        renderer.drawFrame();
    }
}

void Application::cleanup() {
    renderer.cleanup();
}