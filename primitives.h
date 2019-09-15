// Source code by Shawn Halayka
// Source code is in the public domain


#ifndef PRIMITIVES_H
#define PRIMITIVES_H


#include <cmath>


class vertex_3
{
public:
	inline vertex_3(void) : x(0.0f), y(0.0f), z(0.0f) { /*default constructor*/ }
	inline vertex_3(const float src_x, const float src_y, const float src_z) : x(src_x), y(src_y), z(src_z) { /* custom constructor */ }

	inline bool operator==(const vertex_3 &right) const
	{
		if(right.x == x && right.y == y && right.z == z)
			return true;
		else
			return false;
	}

	inline bool operator<(const vertex_3 &right) const
	{
		if(right.x > x)
			return true;
		else if(right.x < x)
			return false;

		if(right.y > y)
			return true;
		else if(right.y < y)
			return false;

		if(right.z > z)
			return true;
		else if(right.z < z)
			return false;

		return false;
	}

	inline const vertex_3& operator-(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = this->x - right.x;
		temp.y = this->y - right.y;
		temp.z = this->z - right.z;

		return temp;
	}

	inline const vertex_3& operator+(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = this->x + right.x;
		temp.y = this->y + right.y;
		temp.z = this->z + right.z;

		return temp;
	}

	inline const vertex_3& operator*(const float &right) const
	{
		static vertex_3 temp;

		temp.x = this->x * right;
		temp.y = this->y * right;
		temp.z = this->z * right;

		return temp;
	}
	
	inline const vertex_3& cross(const vertex_3 &right) const
	{
		static vertex_3 temp;

		temp.x = y*right.z - z*right.y;
		temp.y = z*right.x - x*right.z;
		temp.z = x*right.y - y*right.x;

		return temp;
	}

	inline float dot(const vertex_3 &right) const
	{
		return x*right.x + y*right.y + z*right.z;
	}

	inline const float self_dot(void)
	{
		return x*x + y*y + z*z;
	}

	inline const float length(void)
	{
		return std::sqrt(self_dot());
	}

	inline const void normalize(void)
	{
		float len = length();

		if(0.0f != len)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	float x, y, z;
};

class triangle
{
public:
	vertex_3 vertex[3];
};

class quaternion
{
public:
	inline quaternion(void) : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { /*default constructor*/ }
	inline quaternion(const float src_x, const float src_y, const float src_z, const float src_w) : x(src_x), y(src_y), z(src_z), w(src_w) { /* custom constructor */ }

	float x, y, z, w;
};

#endif
