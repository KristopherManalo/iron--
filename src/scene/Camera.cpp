#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float width, float height)
	: width(width), height(height) {
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::translate(
		glm::mat4(1.0f),
		-glm::vec3(position, 0.0f)
	);
}

glm::mat4 Camera::getProjectionMatrix() const {
	//return glm::ortho(
	//	0.0f,
	//	width,
	//	0.0f,
	//	height,
	//	-1.0f,
	//	1.0f
	//);
	glm::mat4 projection = glm::orthoRH_ZO(
		0.0f, width,
		0.0f, height,
		-1.0f, 1.0f
	);
	//glm::mat4 projection = glm::orthoRH_ZO(
	//	-width / 2.0f, width / 2.0f,
	//	-height / 2.0f, height / 2.0f,
	//	-1.0f, 1.0f
	//);

	//projection[1][1] *= -1.0f;

	return projection;
}
