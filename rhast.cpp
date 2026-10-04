#define GLFW_EXPOSE_NATIVE_WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <string>
#include <sstream>
#include <fstream>
#include <limits>
#include <chrono>

#include "Renderer.h"

/*
	0 -> W
	1 -> A
	2 -> S
	3 -> D
	4 -> Q
	5 -> E
	6 -> SPACE
	7 -> CRTL
*/
bool keysHeld[8] = { false, false, false, false, false, false };

static void Draw(HWND hwnd, const std::vector<unsigned char>& pixels, int width, int height)
{
	HDC hdc = GetDC(hwnd);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;

	int scanedLines = StretchDIBits(
		hdc,
		0, 0, width, height,
		0, 0, width, height,
		pixels.data(),
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
	);

	if (scanedLines == 0)
	{
		throw std::runtime_error("Failed to Display pixels\n");
	}

	ReleaseDC(hwnd, hdc);
} // Draw(HWND hwnd, const std::vector<unsigned char>& pixels, int width, int height)

static void keyCallback(GLFWwindow* window, int key, int seconds, int action, int mods)
{
	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS) keysHeld[0] = true;
		else if (action == GLFW_RELEASE) keysHeld[0] = false;
	}
	else if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS) keysHeld[1] = true;
		else if (action == GLFW_RELEASE) keysHeld[1] = false;
	}
	else if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS) keysHeld[2] = true;
		else if (action == GLFW_RELEASE) keysHeld[2] = false;
	}
	else if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS) keysHeld[3] = true;
		else if (action == GLFW_RELEASE) keysHeld[3] = false;
	}
	else if (key == GLFW_KEY_Q)
	{
		if (action == GLFW_PRESS) keysHeld[4] = true;
		else if (action == GLFW_RELEASE) keysHeld[4] = false;
	}
	else if (key == GLFW_KEY_E)
	{
		if (action == GLFW_PRESS) keysHeld[5] = true;
		else if (action == GLFW_RELEASE) keysHeld[5] = false;
	}
	else if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS) keysHeld[6] = true;
		else if (action == GLFW_RELEASE) keysHeld[6] = false;
	}
	else if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS) keysHeld[7] = true;
		else if (action == GLFW_RELEASE) keysHeld[7] = false;
	}
} // keyCallback(GLFWwindow* window, int key, int seconds, int action, int mods)

int main()
{
	/* --------------- Setting up GLFW Window --------------- */
	GLFWwindow* window;
	GLFWmonitor* monitor;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	window = glfwCreateWindow(mode->width, mode->height, "Test", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, keyCallback); // setting up keybinds
	HWND hwnd = glfwGetWin32Window(window);

	/* --------------- Setting up pixel buffer, z buffer, and Renderer class --------------- */
	Renderer render(0.0, 0.0, -10.0, 60, mode->width, mode->height);
	std::vector<unsigned char> pixels(render.width * render.height * 3);
	std::vector<double> zBuf(render.width * render.height);

	int frames = 0;
	double elapsedTime = 0.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	/* --------------- Main Render Loop --------------- */
	while (!glfwWindowShouldClose(window))
	{
		if (keysHeld[0] == true) render.cam.moveForward();
		if (keysHeld[2] == true) render.cam.moveBackward();
		if (keysHeld[1] == true) render.cam.moveLeft();
		if (keysHeld[3] == true) render.cam.moveRight();
		if (keysHeld[4] == true) render.cam.rotateRight();
		if (keysHeld[5] == true) render.cam.rotateLeft();
		if (keysHeld[6] == true) render.cam.moveUp();
		if (keysHeld[7] == true) render.cam.moveDown();

		// Setting Background to constant color and Z buffer to infinity
		for (int i = 0; i < pixels.size(); i += 3)
		{
			// Little Endian for Windows
			pixels[i] = 0;
			pixels[i + 1] = 0;
			pixels[i + 2] = 0;

			zBuf[i / 3] = std::numeric_limits<double>::infinity();
		}

		std::vector<plane> frustrum = render.cam.getFrustrum();
		std::vector<boolean> frustrumSides = render.cam.getBooleans(frustrum);


		for (unsigned long i = 0; i < render.objects.size(); i++)
		{
			Object curObject = render.objects[i];
			for (unsigned long j = 0; j < curObject.triangles.size(); j++)
			{
				Triangle curTri = curObject.triangles[j];

				Vector los = Vector::computeVec(render.cam.pos, curTri.getCentroid());
				if (Vector::dotProduct(curTri.normal, los) <= 0) continue;

				if (
					render.cam.isPointInFrustrum(curTri.v1, frustrum, frustrumSides) ||
					render.cam.isPointInFrustrum(curTri.v2, frustrum, frustrumSides) ||
					render.cam.isPointInFrustrum(curTri.v3, frustrum, frustrumSides)
					)
				{
					curTri = render.TrianglePixelCoords(curTri);
					render.fillScanLines(pixels, zBuf, curTri, &(curObject.materials[curTri.materialIndex]));
					//std::vector<int> boundingBox = cur.getBoundingBox();
					//render.clipBoundingBox(boundingBox);
					//render.fillBoundingBox(pixels, zBuf, boundingBox, cur);
				}
			}
		}

		Draw(hwnd, pixels, render.width, render.height);
		auto currentTime = std::chrono::high_resolution_clock::now();
		double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
		lastTime = currentTime;

		frames++;
		elapsedTime += deltaTime;

		if (elapsedTime >= 1.0)
		{
			double fps = frames / elapsedTime;
			printf("%lf\n", fps);

			frames = 0;
			elapsedTime = 0.0;
		}

		glfwPollEvents();
	} // triangleList for loop

	glfwTerminate();
	return 0;
} // main()