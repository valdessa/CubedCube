#ifndef INCLUDE_CAMARA_H_
#define INCLUDE_CAMARA_H_ 1

namespace poyo {
	class Camera {
	 public:
		Camera(cFVec3& position, cfloat speed);
		Camera(cFVec3& position, cfloat& pitch, cfloat& yaw, cfloat speed);
		~Camera();

		//Copy Constructor
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

		//Movement Constructor
		Camera(Camera&&) = delete;
		Camera& operator=(Camera&&) = delete;

		void updateCamera(float deltaTime);

		void setPosition(cFVec3& pos);

		cFVec3& getPosition() const;
		cfloat& getPitch() const;
		cfloat& getYaw() const;
		
	 private:
		FVec3 position_;
		FVec3 forward_;
		FVec3 right_;
		FVec3 up_;

		FVec3 worldUp_;

		float pitch_, yaw_;
		float fov_, far_, near_;
		float speed_, sensitivity_;
	};
}



#endif