#pragma once

#undef min
#undef max

#include <algorithm>

#include "Coords.h"

class Triangle
{
public:

	/* --------------- Public Member --------------- */
	Coords v1, v2, v3; // verticies 1, 2, and 3
	Coords vt1, vt2, vt3; // uv/texture coordinates for verticies 1, 2, and 3
	Coords vn1, vn2, vn3; // normal vector for verticies 1, 2, and 3
	Vector normal; // normal vector for the triangle
	int materialIndex;

	/* --------------- Public Constructor --------------- */
	Triangle() : v1(), v2(), v3(), vt1(), vt2(), vt3(), vn1(), vn2(), vn3(), normal() {}
	Triangle(Coords v1, Coords v2, Coords v3, Coords vt1, Coords vt2, Coords vt3, Coords vn1, Coords vn2, Coords vn3, int materialIndex) :
		v1(v1), v2(v2), v3(v3), vt1(vt1), vt2(vt2), vt3(vt3), vn1(vn1), vn2(vn2), vn3(vn3), materialIndex(materialIndex)
	{
		Vector vec1 = Coords::computeVec(v1, v2);
		Vector vec2 = Coords::computeVec(v1, v3);

		this->normal = Vector::crossProduct(vec1, vec2);
	}

	/* --------------- Public Methods --------------- */
	Coords getCentroid()
	{
		return {
			(v1.x + v2.x + v3.x) / 3,
			(v1.y + v2.y + v3.y) / 3,
			(v1.z + v2.z + v3.z) / 3,
		};
	} // getCentroid()

	boolean edgeFunctionBoolean(Vector edge, const Vector test)
	{
		double x = edge.x;

		edge.x = edge.y;
		edge.y = -x;

		double temp = Vector::dotProduct(edge, test);

		if (temp >= 0) return true;
		else return false;
	} //edgeFunctionBoolean(Vector edge, Vector test)

	std::vector<int> getBoundingBox() const
	{
		std::vector<int> ret(4);

		if (std::min({ v1.x, v2.x, v3.x }) == v1.x) ret[0] = (int)v1.x;
		else if (std::min({ v1.x, v2.x, v3.x }) == v2.x) ret[0] = (int)v2.x;
		else ret[0] = (int)v3.x;

		if (std::max({ v1.x, v2.x, v3.x }) == v1.x) ret[1] = (int)v1.x;
		else if (std::max({ v1.x, v2.x, v3.x }) == v2.x) ret[1] = (int)v2.x;
		else ret[1] = (int)v3.x;

		if (std::min({ v1.y, v2.y, v3.y }) == v1.y) ret[2] = (int)v1.y;
		else if (std::min({ v1.y, v2.y, v3.y }) == v2.y) ret[2] = (int)v2.y;
		else ret[2] = (int)v3.y;

		if (std::max({ v1.y, v2.y, v3.y }) == v1.y) ret[3] = (int)v1.y;
		else if (std::max({ v1.y, v2.y, v3.y }) == v2.y) ret[3] = (int)v2.y;
		else ret[3] = (int)v3.y;

		return ret;
	} // getBoundingBox()

	boolean inTriangle(const Coords point, const std::vector<Vector> triVec)
	{
		boolean e1 = edgeFunctionBoolean(triVec[0], Vector::computeVec(v1, point));
		boolean e2 = edgeFunctionBoolean(triVec[1], Vector::computeVec(v2, point));
		boolean e3 = edgeFunctionBoolean(triVec[2], Vector::computeVec(v3, point));

		return e1 & e2 & e3;
	} // inTriangle(Coords point)

	std::vector<Coords> getPointsBotToTop() const
	{
		std::vector<Coords> ret = { v1, v2, v3 };
		std::sort(ret.begin(), ret.end(),
			[](const Coords& a, const Coords& b) { return a.y < b.y; });
		return ret;

	} // getPointsBotToTop()
};