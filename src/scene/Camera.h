#pragma once

#include <glm/glm.hpp>

class Camera {
public:
	glm::vec2 position{ 0.0f, 0.0f };
	float rotation = 0.0f;
	float zoom = 1.0f;
	Camera(
		float width,
		float height
	);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
private:
	float width;
	float height;
};