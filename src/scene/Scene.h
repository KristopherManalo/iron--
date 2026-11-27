#pragma once

class Scene {
public:
	virtual ~Scene() = default;

	virtual void initialize() = 0;
	virtual void update(float deltaTime) = 0;

private:

};

