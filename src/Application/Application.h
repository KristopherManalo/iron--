#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/Renderer.h"

class Application {
public:
	void run();
	void cleanup();
private:
	const char* PROGRAM_NAME = "iron--";
	Renderer renderer;

	void setWindowIcon(
		GLFWwindow*,
		const char*
	);

	void mainLoop();
};