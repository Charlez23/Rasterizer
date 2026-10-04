#pragma once
typedef class Coords
{
public:
	using Vector = Coords;

	/* --------------- Public Member --------------- */
	double x; // x coordinate
	double y; // y coordinate
	double z; // z coordinate

	/* --------------- Operator Overloading --------------- */
	bool operator==(const Coords& other) const
	{
		return this->x == other.x && this->y == other.y && this->z == other.z;
	}

	/* --------------- Public Constructor --------------- */
	Coords() : x(0), y(0), z(0) {}
	Coords(double a1, double a2, double a3) : x(a1), y(a2), z(a3) {}

	/* --------------- Public Static Methods --------------- */
	static Vector computeVec(const Coords p1, const Coords p2)
	{
		return {
			p2.x - p1.x,
			p2.y - p1.y,
			p2.z - p1.z
		};
	} // computeVec(Coords p1, Coords p2)

	static double dotProduct(const Vector a, const Vector b)
	{
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	} // dotProduct(Vector a, Vector b)

	static Vector crossProduct(Vector v1, Vector v2)
	{
		return {
			(v1.y * v2.z) - (v1.z * v2.y),
			-(v1.x * v2.z) + (v1.z * v2.x),
			(v1.x * v2.y) - (v1.y * v2.x)
		};
	} // crossProduct(Vector v1, Vector v2)
} Vector;

class plane
{
public:

	/* --------------- Public Members --------------- */
	double a;
	double b;
	double c;
	double d;

	/* --------------- Public Constructors --------------- */
	plane() : a(0), b(0), c(0), d(0) {}
	plane(double a, double b, double c, double d) : a(a), b(b), c(c), d(d) {}
};

class Line2D
{
public:

	/* --------------- Public Members --------------- */
	double a;
	double b;
	double c;

	/* --------------- Public Constructors --------------- */
	Line2D() : a(0), b(0), c(0) {}
	Line2D(double a, double b, double c) : a(a), b(b), c(c) {}

	/* --------------- Public Static Methods --------------- */
	static Line2D constructLine(const Coords& p1, const Coords& p2)
	{
		double a = p1.y - p2.y;
		double b = p2.x - p1.x;
		double c = (a * p1.x) + (b * p1.y);

		return { a, b, c };
	}
};