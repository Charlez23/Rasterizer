#pragma once

#include <vector>
#include <cmath>

#include "Triangle.h"
#include "Camera.h"
#include "Object.h"
#include "LightSource.h"

/* --------------- Public Members --------------- */
class Renderer {
	public:
		std::vector<Object> objects;
		std::vector<LightSource> lights;
		LightSource ambientLightSource;
		Camera cam;
		int width;
		int height;		

		/* --------------- Public Constructor --------------- */
		Renderer(double x, double y, double z, int fovY, int width, int height) : cam(x, y, z, fovY)
		{
			this->width = width;
			this->height = height;

			objects = {};
			objects.push_back(Object("cube.obj"));

			lights = {};
			ambientLightSource = LightSource({0.0, 10.0, -10.0}, 255, 0, 0, 1.0, 1.0, 1.0);
		}

		/* --------------- Public Methods --------------- */
		Coords worldToPixelCoords(Coords vertex)
		{
			double maxYInImagePlane = std::tan(cam.fovY / 2);
			double maxXInImagePlane = maxYInImagePlane * cam.aspectRatio;

			Coords coordsRelativeToCamera = {
				vertex.x - cam.pos.x,
				vertex.y - cam.pos.y,
				vertex.z - cam.pos.z
			};
			
			double camX = Vector::dotProduct(coordsRelativeToCamera, cam.right);
			double camY = Vector::dotProduct(coordsRelativeToCamera, cam.up);
			double camZ = Vector::dotProduct(coordsRelativeToCamera, cam.forward);

			Coords temp = {};
			temp.x = camX / camZ / maxXInImagePlane;
			temp.y = camY / camZ / maxYInImagePlane;
			temp.z = coordsRelativeToCamera.z;

			temp.y = (1.0 - temp.y) / 2 * height;
			temp.x = (1.0 + temp.x) / 2 * width;

			return temp;
		}

		/*
		* Rotates a point in 2D space 
		*/
		Coords rotateVertex2D(Coords vertex)
		{
			double x = vertex.x;
			double y = vertex.y;

			vertex.x = x * std::cos(0.1) - y * std::sin(0.1);
			vertex.y = x * std::sin(0.1) + y * std::cos(0.1);

			return vertex;
		} // rotateVertex2D(Coords vertex)

		Triangle rotateTriangle(Triangle tri)
		{
			tri.v1 = rotateVertex2D(tri.v1);
			tri.v2 = rotateVertex2D(tri.v2);
			tri.v3 = rotateVertex2D(tri.v3);

			return tri;
		} // rotateTriangle(Triangle tri)

		Triangle TrianglePixelCoords(Triangle tri)
		{
			Triangle temp = tri;
			temp.v1 = worldToPixelCoords(tri.v1);
			temp.v2 = worldToPixelCoords(tri.v2);
			temp.v3 = worldToPixelCoords(tri.v3);

			return temp;
		} // TrianglePixelCoords(Triangle tri)

		double signedArea(Coords p1, Coords p2, Coords p3)
		{
			return 0.5 * ( (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y) );
		} // signedArea(Coords p1, Coords p2, Coords p3)

		void fillBoundingBox(std::vector<unsigned char>& pixels, std::vector<double>& zBuf, std::vector<int> box, Triangle tri)
		{
			std::ifstream texture("cubeTexture.bmp", std::ios::binary);
			if (!texture)
			{
				throw std::runtime_error("Failed to open texture file\n");
			}

			int offset = 0; // offset to move straight to pixel data
			texture.seekg(10, std::ios::beg); // move file pointer to place where data points to pixel data
			texture.read((char*)(&offset), 4);

			int textureWidth = 0;
			texture.seekg(18, std::ios::beg); // move file pointer to place where data is about width
			texture.read((char*)(&textureWidth), 4);

			int textureHeight = 0;
			texture.seekg(22, std::ios::beg); // move file pointer to place where data is about height
			texture.read((char*)(&textureHeight), 4);

			short bitsPerPixel = 0;
			texture.seekg(28, std::ios::beg); // move file pointer to place where data talks about number of bits for a pixel
			texture.read((char*)(&bitsPerPixel), 2);

			for (int y = box[2]; y < box[3]; y++)
			{
				for (int x = box[0]; x < box[1]; x++)
				{
					Coords temp = { (double)x, (double)y, 0.0 };
					std::vector<Vector> triVec = {
						Vector::computeVec(tri.v1, tri.v2),
						Vector::computeVec(tri.v2, tri.v3),
						Vector::computeVec(tri.v3, tri.v1)
					};

					if (tri.inTriangle(temp, triVec))
					{
						// Calculating coefficients for barycentric coordinates
						double signedAreaTri = signedArea(tri.v1, tri.v2, tri.v3);
						if (std::abs(signedAreaTri) < 1e-8) continue;

						double l1 = signedArea(temp, tri.v2, tri.v3) / signedAreaTri;
						double l2 = signedArea(tri.v1, temp, tri.v3) / signedAreaTri;
						double l3 = signedArea(tri.v1, tri.v2, temp) / signedAreaTri;

						// Interpolating z depth, then doing occulsion culling based on z buffer
						double z = l1 * tri.v1.z + l2 * tri.v2.z + l3 * tri.v3.z;
						if (z >= zBuf[y * width + x]) continue;
						else zBuf[y * width + x] = z;

						Coords uv = {
							l1 * tri.vt1.x + l2 * tri.vt2.x + l3 * tri.vt3.x,
							l1* tri.vt1.y + l2 * tri.vt2.y + l3 * tri.vt3.y,
							0.0
						};

						int texX = (int)(uv.x * textureWidth);
						int texY = (int)(uv.y * textureHeight);

						uint8_t bgr[3] = {}; // I currently don't support alpha values
						texture.seekg(offset + (texY * textureWidth * bitsPerPixel / 8) + texX * bitsPerPixel / 8);
						texture.read(reinterpret_cast<char*>(bgr), 3);

						pixels[(y * width + x) * 3] = bgr[0];
						pixels[(y * width + x) * 3 + 1] = bgr[1];
						pixels[(y * width + x) * 3 + 2] = bgr[2];
					} // tri.inTriangle(temp) == true
				} // x for loop
			} // y for loop

			texture.close();
			return;
		} // fillBoundingBox(std::vector<unsigned char>& pixels, std::vector<double>& zBuf, std::vector<double> box, Triangle tri)

		void clipBoundingBox(std::vector<int>& box) const
		{
			if (box[0] < 0) box[0] = 0;
			if (box[1] > width) box[1] = width;
			if (box[2] < 0) box[2] = 0;
			if (box[3] > height) box[3] = height;
			return;
		} // clipBoundingBox(std::vector<double>& box)

		void fillLine(std::vector<unsigned char>& pixels, std::vector<double>& zBuf, const Triangle tri, const materialInfoStruct* material, const double y, const double startX, const double endX)
		{
			int py = (int)std::floor(y);
			if (py < 0 || py >= height) return;

			for (double x = startX; x < endX; x++)
			{
				int px = (int)std::floor(x);
				if (px < 0 || px >= width) continue;

				// Calculating coefficients for barycentric coordinates
				double signedAreaTri = signedArea(tri.v1, tri.v2, tri.v3);
				if (std::abs(signedAreaTri) < 1e-9) continue;

				Coords temp = { x, y, 0 };
				double l1 = signedArea(temp, tri.v2, tri.v3) / signedAreaTri;
				double l2 = signedArea(tri.v1, temp, tri.v3) / signedAreaTri;
				double l3 = signedArea(tri.v1, tri.v2, temp) / signedAreaTri;

				double z = l1 * tri.v1.z + l2 * tri.v2.z + l3 * tri.v3.z;
				if (z >= zBuf[py * width + px]) continue;
				else zBuf[py * width + px] = z;

				Coords uv = {
					l1 * tri.vt1.x + l2 * tri.vt2.x + l3 * tri.vt3.x,
					l1 * tri.vt1.y + l2 * tri.vt2.y + l3 * tri.vt3.y,
					0.0
				};

				double u = std::clamp(uv.x, 0.0, 1.0);
				double v = std::clamp(uv.y, 0.0, 1.0);

				int textureWidth = material->textureInfo.textureWidth;
				int textureHeight = material->textureInfo.textureHeight;

				int texX = (int)(u * (textureWidth - 1));
				int texY = (int)(v * (textureHeight - 1));

				int bTex = material->texture[(texY * textureWidth + texX) * 3];
				int gTex = material->texture[(texY * textureWidth + texX) * 3 + 1];
				int rTex = material->texture[(texY * textureWidth + texX) * 3 + 2];
				
				pixels[(py * width + px) * 3] = (unsigned char)(bTex * material->Ka.b * (1.0 + ((lights[0].b * lights[0].bI - bTex) / 255.0)));
				pixels[(py * width + px) * 3 + 1] = (unsigned char)(gTex * material->Ka.g * (1.0 + ((lights[0].g * lights[0].gI - gTex) / 255.0)));
				pixels[(py * width + px) * 3 + 2] = (unsigned char)(rTex * material->Ka.r * (1.0 + ((lights[0].r * lights[0].rI - rTex) / 255.0)));
				
			}

		} // fillLine(std::vector<unsigned char>& pixels, double start, double end)

		void fillScanLines(std::vector<unsigned char>& pixels, std::vector<double>& zBuf, const Triangle tri, const materialInfoStruct* materialStruct)
		{
			std::vector<Coords> points = tri.getPointsBotToTop();	

			Line2D main = Line2D::constructLine(points[0], points[2]);
			Line2D bot = Line2D::constructLine(points[0], points[1]);
			Line2D top = Line2D::constructLine(points[1], points[2]);

			// true means point[1] is on the left side of the line; false means right
			bool left = (main.a * points[1].x) + (main.b * points[1].y) < main.c ? false : true;

			// Horizontal Line Case
			if (bot.a == 0)
			{
				for (double y = points[0].y + 1; y < points[2].y; y++)
				{
					double startX;
					double endX;

					if (left)
					{
						startX = (top.c - top.b * y) / top.a;
						endX = (main.c - main.b * y) / main.a;
						
					}
					else
					{
						startX = (main.c - main.b * y) / main.a;
						endX = (top.c - top.b * y) / top.a;
					}

					fillLine(pixels, zBuf, tri, materialStruct, y, startX, endX);
				}

				return;
			}
			else if (top.a == 0)
			{
				for (double y = points[0].y + 1; y < points[2].y; y++)
				{
					double startX;
					double endX;

					if (left)
					{
						startX = (bot.c - bot.b * y) / bot.a;
						endX = (main.c - main.b * y) / main.a;

					}
					else
					{
						startX = (main.c - main.b * y) / main.a;
						endX = (bot.c - bot.b * y) / bot.a;
					}

					fillLine(pixels, zBuf, tri, materialStruct, y, startX, endX);
				}

				return;
			}


			double yIntersection = points[1].y;
			for (double y = points[0].y; y <= points[2].y; y++)
			{
				double startX;
				double endX;

				if (y < yIntersection)
				{
					if (left)
					{
						startX = (bot.c - bot.b * y) / bot.a;
						endX = (main.c - main.b * y) / main.a;
					}
					else
					{
						startX = (main.c - main.b * y) / main.a;
						endX = (bot.c - bot.b * y) / bot.a;
					}
				}
				else
				{
					if (left)
					{
						startX = (top.c - top.b * y) / top.a;
						endX = (main.c - main.b * y) / main.a;
					}
					else
					{
						startX = (main.c - main.b * y) / main.a;
						endX = (top.c - top.b * y) / top.a;
					}
				}

				fillLine(pixels, zBuf, tri, materialStruct, y, startX, endX);
			}

			return;
		} // fillScanLines(std::vector<unsigned char>& pixels, Triangle tri)
};