#ifndef INCLUDE_CAMARA_H_
#define INCLUDE_CAMARA_H_ 1

#include "glm.hpp"

class Camera {
 public:
	Camera(const glm::vec3& position);
	~Camera();

	//Copy Constructor
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	//Movement Constructor
	Camera(Camera&&) = delete;
	Camera& operator=(Camera&&) = delete;

	void updateCamera(float deltaTime);

	glm::vec3 position_;
	glm::vec3 forward_;
	glm::vec3 right_;
	glm::vec3 up_;

	glm::vec3 worldUp_;

	float pitch_, yaw_;
	float fov_, far_;
	float speed_, sensitivity_;
};


#endif