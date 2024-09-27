#ifndef INCLUDE_CAMARA_H_
#define INCLUDE_CAMARA_H_ 1

namespace poyo {
	class Camera {
	 public:
		Camera(cFVec3& position);
		~Camera();

		//Copy Constructor
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

		//Movement Constructor
		Camera(Camera&&) = delete;
		Camera& operator=(Camera&&) = delete;

		void updateCamera(float deltaTime);

		FVec3 position_;
		FVec3 forward_;
		FVec3 right_;
		FVec3 up_;

		FVec3 worldUp_;

		float pitch_, yaw_;
		float fov_, far_;
		float speed_, sensitivity_;
	};
}



#endif