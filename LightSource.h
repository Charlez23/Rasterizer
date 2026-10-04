#include "Coords.h"

#pragma once

class LightSource
{
public:
	Coords pos;
	int r;
	int g;
	int b;
	double rI;
	double gI;
	double bI;

	LightSource()
	{
		r = 0;
		g = 0;
		b = 0;
		rI = 0.0;
		gI = 0.0;
		bI = 0.0;
	}
	LightSource(Coords pos, int r, int g, int b, double rI, double gI, double bI) : pos(pos), r(r), g(g), b(b), rI(rI), gI(gI), bI(bI) {}
};