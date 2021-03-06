#include "Vector.h"

#include <cmath>


Vector2::Vector2(double _x, double _y) : x(_x), y(_y)
{
}


double& Vector2::operator[](unsigned index)
{
	if (index > 1)
		return arr[0];

	return arr[index];
}

double Vector2::operator[](unsigned index) const
{
	if (index > 1)
		return arr[0];

	return arr[index];
}

Vector2 Vector2::operator+(const Vector2 & rhs) const
{
	return Vector2(x + rhs.x, y + rhs.y);
}

Vector2 Vector2::operator-(const Vector2 & rhs) const
{
	return Vector2(x - rhs.x, y - rhs.y);
}

Vector2 Vector2::operator*(const float rhs) const
{
	return Vector2(x * rhs, y * rhs);
}

Vector2 Vector2::operator/(const float rhs) const
{
	if (rhs == 0.f )
		return Vector2();
	return Vector2(x / rhs, y/rhs);
}

void Vector2::operator+=(const Vector2 & rhs)
{
	x += rhs.x;
	y += rhs.y;
}

void Vector2::operator-=(const Vector2 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vector2::operator*=(const double rhs)
{
	x *= rhs;
	y *= rhs;
}

void Vector2::operator/=(const double rhs)
{
	if (rhs != 0)
	{
		x /= rhs;
		y /= rhs;
	}
}

bool Vector2::operator==(const Vector2 & rhs) const
{
	if (x == rhs.x && y == rhs.y)
		return true;
	else
		return false;
}

bool Vector2::operator!=(const Vector2 & rhs) const
{
	return !(*this == rhs);
}

double Vector2::Dot(const Vector2 & rhs) const
{
	return x * rhs.x + y * rhs.y;
}

double Vector2::Length(void) const
{
	return std::sqrt(Dot(*this));
}
