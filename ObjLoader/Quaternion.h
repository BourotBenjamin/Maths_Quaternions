#pragma once
#include <math.h>
#include "glm.hpp"
class Quaternion {
private:
	float x, y, z, w;

public:
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
		return q;
	}

	void normalize() {
		float len = this->length();
		this->x = this->x / len;
		this->y = this->y / len;
		this->z = this->y / len;
		this->w = this->w / len;
		return q;
	}

	void conjugate() {
		q.x = -q.x;
		q.y = -q.y;
		q.z = -q.z;
		return q;
	}

	glm::mat4 toMatrixUnit() {
		glm::mat4 ret(
			( 1 - 2 * this->y * this->y - 2 * this->z * this->z ),
			( 2 * this->x * this->y - 2 * this->w * this->z ),
			( 2 * this->x * this->z + 2 * this->w + this->y ),
			0,

			( 2 * this->x * this->y + 2 * this->w * this->z ),
			( 1 - 2 * this->x * this->x - 2 * this->z * this->z ),
			( 2 * this->y * this->z + 2 * this->w * this->x ),
			0,

			(  2 * this->x * this->z - 2 * this->w * this->z ),
			( 2 * this->y * this->z - 2 * this->w * this->x ),
			( 1 - 2 * this->x * this->x - 2 * this->y * this->y ),
			0,

			0,
			0,
			0,
			1
		);
		return ret;
	}
}

 /*
public static Quaternion mult(Quaternion A, Quaternion B, Quaternion C) {
	C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
	C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
	C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
	C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
	return C;
}*/

