#include <iostream>
#include <filesystem>
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION 
#include "stb/stb_image.h"

#include "config.h"

void Application::run() {
#ifndef NDEBUG
	std::cout << "Hello world\n";
#endif
	renderer.initialize(PROGRAM_NAME);
    setWindowIcon(renderer.getWindow(), "assets/mambo.png");
	mainLoop();

    cleanup();
}

void Application::setWindowIcon(
    GLFWwindow* window,
    const char* iconPath
) {
    GLFWimage images[1];
    int channels;

	//std::cout << "Working directory: " << std::filesystem::current_path() << "\n";
    images[0].pixels
        = stbi_load(
            iconPath,
            &images[0].width,
            &images[0].height,
            &channels,
            STBI_rgb_alpha
        );
    if (!images[0].pixels) {
        std::cerr << "Failed to load sprite" << std::endl;
        printf("stbi_load failed: %s\n", stbi_failure_reason());
        stbi_image_free(images[0].pixels);
        return;
    }

    if (images[0].pixels) {
        glfwSetWindowIcon(renderer.getWindow(), 1, images);
        stbi_image_free(images[0].pixels);
    }
}

void Application::mainLoop() {
    std::cout << "Press spacebar to quit\n" << std::endl;

#ifdef IN_TITLE_SHOW_FPS
	auto lastTime = std::chrono::steady_clock::now();
    int frameCount = 0;
#endif
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        glfwPollEvents();
#define getkeypress(key) (glfwGetKey(renderer.getWindow(), key) == GLFW_PRESS)
        if (getkeypress(GLFW_KEY_SPACE)) break;
        if (getkeypress(GLFW_KEY_W)) {
            std::cout << "HOLDING W :3" << std::endl;
            renderer.camera.position.y += 0.01;
        }
        if (getkeypress(GLFW_KEY_A)) {
            std::cout << "HOLDING A :3" << std::endl;
            renderer.camera.position.x -= 0.01;
        }
        if (getkeypress(GLFW_KEY_S)) {
            std::cout << "HOLDING S :3" << std::endl;
            renderer.camera.position.y -= 0.01;
        }
        if (getkeypress(GLFW_KEY_D)) {
            std::cout << "HOLDING D :3" << std::endl;
            renderer.camera.position.x += 0.01;
        }

#ifndef NDEBUG
		if (renderer.framebufferResized) {
			renderer.framebufferResized = false;
			renderer.recreateSwapChain();
            std::cout << "WAS RESIZED :3" << std::endl;
		}
#endif

        renderer.drawFrame();

#ifdef IN_TITLE_SHOW_FPS
        frameCount++;

        auto currentTime = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(
            currentTime - lastTime
        ).count();

        if (elapsed >= 0.5f) {
            int fps = static_cast<int>(frameCount / elapsed);

            glfwSetWindowTitle(
                renderer.getWindow(),
                std::format("{} | FPS: {}", PROGRAM_NAME, fps).c_str()
            );

            frameCount = 0;
            lastTime = currentTime;
        }
#endif

    }
}

void Application::cleanup() {
    renderer.cleanup();
}