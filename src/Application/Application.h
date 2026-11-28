#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/Renderer.h"

/// @brief Application entry point
class Application {
public:
	/// @brief Runs program
	void run();
	/// @brief 
	void cleanup();

	/// @brief Sets window icon.
	/// @param window Window of application
	/// @param iconPath Path to image file
	void setWindowIcon(
		GLFWwindow* window,
		const char* iconPath
	);
private:
	const char* PROGRAM_NAME = "iron--";
	Renderer renderer;


	void mainLoop();
};