#pragma once
#include "Coords.h"

class Camera
{
public:

	/* --------------- Public Member --------------- */
	Coords pos; // position of the camera
	Vector forward; // vector which represents the forward direction
	Vector right; // vector which represents the right direction
	Vector up; // vector which represents the up direction
	double fovY; // angle in radians for observable area in the y direction
	double aspectRatio; // x : y

	/* --------------- Public Constructors --------------- */
	Camera()
	{
		pos.x = 0.0;
		pos.y = 0.0;
		pos.z = 0.0;
		fovY = 0.0;
		aspectRatio = 0.0;
		forward = { 0.0, 0.0, 1.0 };
		right = { 1.0, 0.0, 0.0 };
		up = { 0.0, 1.0, 0.0 };
	}

	Camera(double x, double y, double z, int fovY)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->pos.z = z;
		this->fovY = fovY * 3.14159265 / 180; // converting to radians
		aspectRatio = 16.0 / 10.0;

		forward = { 0, 0, 1 };
		right = { 1, 0, 0 };
		up = { 0, 1, 0 };
	};

	/* --------------- Public Methods --------------- */
	void moveForward()
	{
		pos.x += forward.x;
		pos.y += forward.y;
		pos.z += forward.z;
	} // moveForward()

	void moveBackward()
	{
		pos.x -= forward.x;
		pos.y -= forward.y;
		pos.z -= forward.z;
	} // moveBackward()

	void moveLeft()
	{
		pos.x -= right.x;
		pos.y -= right.y;
		pos.z -= right.z;
	} // moveLeft()

	void moveRight()
	{
		pos.x += right.x;
		pos.y += right.y;
		pos.z += right.z;
	} // moveRight()

	void moveUp()
	{
		pos.x += up.x;
		pos.y += up.y;
		pos.z += up.z;
	} // moveUp()

	void moveDown()
	{
		pos.x -= up.x;
		pos.y -= up.y;
		pos.z -= up.z;
	} // moveDown()

	void rotateRight()
	{
		double xf = forward.x;
		double zf = forward.z;
		double xr = right.x;
		double zr = right.z;

		forward.x = xf * std::cos(0.1) - zf * std::sin(0.1);
		forward.z = xf * std::sin(0.1) + zf * std::cos(0.1);
		right.x = xr * std::cos(0.1) - zr * std::sin(0.1);
		right.z = xr * std::sin(0.1) + zr * std::cos(0.1);
	} // rotateRight()

	void rotateLeft()
	{
		double xf = forward.x;
		double zf = forward.z;
		double xr = right.x;
		double zr = right.z;

		forward.x = xf * std::cos(0.1) + zf * std::sin(0.1);
		forward.z = xf * -std::sin(0.1) + zf * std::cos(0.1);
		right.x = xr * std::cos(0.1) + zr * std::sin(0.1);
		right.z = xr * -std::sin(0.1) + zr * std::cos(0.1);
	} // rotateLeft()

	void rotateUp()
	{
		double yf = forward.x;
		double zf = forward.y;
		double yu = up.x;
		double zu = up.y;

		forward.y = yf * std::cos(0.1) - zf * std::sin(0.1);
		forward.z = yf * std::sin(0.1) + zf * std::cos(0.1);
		up.y = yu * std::cos(0.1) - zu * std::sin(0.1);
		up.z = yu * std::sin(0.1) + zu * std::cos(0.1);
	} // rotateUp();

	void rotateDown()
	{
		double yf = forward.x;
		double zf = forward.y;
		double yu = up.x;
		double zu = up.y;

		forward.y = yf * std::cos(0.1) + zf * std::sin(0.1);
		forward.z = yf * -std::sin(0.1) + zf * std::cos(0.1);
		up.y = yu * std::cos(0.1) + zu * std::sin(0.1);
		up.z = yu * -std::sin(0.1) + zu * std::cos(0.1);
	} // rotateDown()

	Vector rotatePlane(const Vector vec, const Vector axis, const double theta)
	{
		// Rodrigues Rotation Formula
		Vector ret = vec;;
		ret.x *= std::cos(theta);
		ret.y *= std::cos(theta);
		ret.z *= std::cos(theta);

		Vector k = axis;
		double norm = std::sqrt(Vector::dotProduct(k, k));
		k.x /= norm;
		k.y /= norm;
		k.z /= norm;

		
		Vector kvCross = Vector::crossProduct(k, vec);
		kvCross.x *= std::sin(theta);
		kvCross.y *= std::sin(theta);
		kvCross.z *= std::sin(theta);

		double kvDot = Vector::dotProduct(k, vec);
		Vector kcos = {
			k.x * kvDot,
			k.y * kvDot,
			k.z * kvDot
		};
		kcos.x *= 1 - std::cos(theta);
		kcos.y *= 1 - std::cos(theta);
		kcos.z *= 1 - std::cos(theta);

		ret.x += kvCross.x + kcos.x;
		ret.y += kvCross.y + kcos.y;
		ret.z += kvCross.z + kcos.z;

		return ret;
	} // rotatePlane(Vector vec, Vector axis, double theta)

	std::vector<plane> getFrustrum()
	{
		std::vector<plane> frustrum(6);
		/*
			frustrum[0] = near plane
			frustrum[1] = far plane
			frustrum[2] = left plane
			frustrum[3] = right plane
			frustrum[4] = bottom plane
			frustrum[5] = top plane
		*/

		Vector vec = forward;
		Coords pointOnPlane = {
			pos.x + vec.x,
			pos.y + vec.y,
			pos.z + vec.z
		};
		double d = Vector::dotProduct(vec, pointOnPlane);
		frustrum[0] = { vec.x, vec.y, vec.z, d };

		pointOnPlane = {
			pos.x + vec.x * 100.0,
			pos.y + vec.y * 100.0,
			pos.z + vec.z * 100.0
		};
		d = Vector::dotProduct(forward, pointOnPlane);
		frustrum[1] = { forward.x, forward.y, forward.z, d };
		
		double fovXHalf = std::atan(aspectRatio * std::tan(fovY / 2));
		vec = rotatePlane(right, up, fovXHalf - 1.570796327);
		d = Vector::dotProduct(vec, pos);
		frustrum[2] = {vec.x, vec.y, vec.z, d};

		vec = rotatePlane(right, up, 1.570796327 - fovXHalf);
		d = Vector::dotProduct(vec, pos);
		frustrum[3] = { vec.x, vec.y, vec.z, d };

		vec = rotatePlane(up, right, fovY / 2);
		d = Vector::dotProduct(vec, pos);
		frustrum[4] = { vec.x, vec.y, vec.z, d };

		vec = rotatePlane(up, right, -fovY / 2);
		d = Vector::dotProduct(vec, pos);
		frustrum[5] = { vec.x, vec.y, vec.z, d };

		return frustrum;
	} // getFrustrum()

	std::vector<boolean> getBooleans(const std::vector<plane> frustrumArg)
	{
		Coords testPoint = {
			pos.x + forward.x * 2.0,
			pos.y + forward.y * 2.0,
			pos.z + forward.z * 2.0
		};

		std::vector<plane> frustrum = frustrumArg;
		std::vector<boolean> ret(6);

		for (int i = 0; i < 6; i++)
		{
			if ((frustrum[i].a * testPoint.x) + (frustrum[i].b * testPoint.y) + (frustrum[i].c * testPoint.z) > frustrum[i].d)
				ret[i] = true;
			else
				ret[i] = false;
		}

		return ret;
	} //  getBooleans(std::vector<plane> frustrumArg)

	boolean isPointInFrustrum(const Coords vertex, const std::vector<plane> frustrum, const std::vector<boolean> sideCheck)
	{
		for (int i = 0; i < 6; i++)
		{
			boolean side = (frustrum[i].a * vertex.x) + (frustrum[i].b * vertex.y) + (frustrum[i].c * vertex.z) > frustrum[i].d;
			if (side != sideCheck[i]) return false;
		}

		return true;
	} // isPointInFrustrum(Coords vertex, std::vector<plane> frustrum, std::vector<boolean> sideCheck)
};