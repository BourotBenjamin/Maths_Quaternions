#pragma once
#include <math.h>
#include "glm.hpp"
class Quaternion {
private:

public:
	float x, y, z, w;
	Quaternion() {
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
		this->w = 1.0f;
	}

	Quaternion(glm::vec3 v, float w) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = w;
	}

	Quaternion(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Quaternion(Quaternion& other) {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		this->w = other.w;
	}

	float length() {
		return (float) sqrt(x * x + y * y + z * z + w * w);
	}


	void fromAxisAngle(glm::vec3 axis, float angle) {
		float sinAngle2 = (float) sin(angle / 2.0);
		this->x = axis.x * sinAngle2;
		this->y = axis.y * sinAngle2;
		this->z = axis.z * sinAngle2;
		this->w = (float) cos(angle / 2.0);
	}

	void normalize() {
		float len = this->length();
		this->x = this->x / len;
		this->y = this->y / len;
		this->z = this->y / len;
		this->w = this->w / len;
	}

	void conjugate() {
		this->x = this->x;
		this->y = -this->y;
		this->z = -this->z;
	}

	glm::mat4 toMatrixUnit() {
		//this->normalize();
		float qxx = this->x * this->x;
		float qyy = this->y * this->y;
		float qzz = this->z * this->z;
		float qxz = this->x * this->x;
		float qxy = this->x * this->y;
		float qyz = this->y * this->z;
		float qwx = this->w * this->x;
		float qwy = this->w * this->y;
		float qwz = this->w * this->z;

		glm::mat4 result(
			(1 - 2 * (qyy + qzz)),
			(2 * (qxy + qwz)),
			(2 * (qxz - qwy)),
			0,

			(2 * (qxy - qwz)),
			(1 - 2 * (qxx + qzz)),
			(2 * (qyz + qwx)),
			0,

			(2 * (qxz + qwy)),
			(2 * (qyz - qwx)),
			(1 - 2 * (qxx + qyy)),
			0,

			0, 0, 0, 1
		);
		return result;
	}

	static Quaternion multiply(Quaternion A, Quaternion B)
	{
		Quaternion C;
		C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
		C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
		C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
		C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
		return C;
	}
};


