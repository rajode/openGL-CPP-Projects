#define _USE_MATH_DEFINES
using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"
#include <map>


struct State {
	int recursions = 0;
	int scene = 0;
	bool operator == (State const& other) const {
		return recursions == other.recursions && scene == other.scene;
	};
};
float sqrt3_4 = sqrt(3) / 4;
float MAX_X = 0.5;
float MIN_X = -0.5;
float MIN_Y = -sqrt3_4;
float MAX_Y = sqrt3_4;



// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_REPEAT || action == GLFW_PRESS) {
			if (key == GLFW_KEY_R) {
				shader.recompile();
			}
			if (key == GLFW_KEY_LEFT) {
				if (state.recursions > 0) {
					state.recursions--;
				}
			}
			if (key == GLFW_KEY_RIGHT) {
				if (state.recursions < 20) {
					state.recursions++;
				}
			}
			if (key == GLFW_KEY_DOWN) {
				if (state.scene > 0) {
					state.scene--;
					state.recursions = 0;
				}
			}
			if (key == GLFW_KEY_UP) {
				if (state.scene < 3) {
					state.scene++;
					state.recursions = 0;
				}
			}
		}
	}
	State getState() {
		return state;
		
	};


private:
	ShaderProgram& shader;
	State state;
};

glm::vec3 randomColor() {
	float c1 = ((0.9) * ((((float)rand()) / (float)RAND_MAX)) + 0.1);
	float c2 = (0.9) * ((((float)rand()) / (float)RAND_MAX)) + 0.1;
	float c3 = (0.9) * ((((float)rand()) / (float)RAND_MAX)) + 0.1;
	return  glm::vec3(c1, c2, c3);
}

void sierpinski(CPU_Geometry& cpuGeomTri, glm::vec3 a, glm::vec3 b, glm::vec3 c, int recursions)
{
	//mid points of the 3 sides of current triangle
	glm::vec3 d = glm::vec3((a.x + c.x) / 2, (a.y + c.y) / 2, 0.f);
	glm::vec3 e = glm::vec3((b.x + c.x) / 2, (b.y + c.y) / 2, 0.f);
	glm::vec3 f = glm::vec3((a.x + b.x) / 2, (a.y + b.y) / 2, 0.f);

	if (recursions == 0) {
		float col1 = (MAX_Y + c.y)/ (2*MAX_Y);
		float col2 = (MAX_X + b.x) / (2*MAX_X);
		float col3 = (MAX_X - a.x) / (2 * MAX_X);
		cpuGeomTri.verts.push_back(a);
		cpuGeomTri.verts.push_back(b);
		cpuGeomTri.verts.push_back(c);
		cpuGeomTri.cols.push_back(glm::vec3(col1,col2,col3));
		cpuGeomTri.cols.push_back(glm::vec3(col1, col2, col3));
		cpuGeomTri.cols.push_back(glm::vec3(col1, col2, col3));
	}
	else
	{
		sierpinski(cpuGeomTri, d, e, c, recursions - 1);
		sierpinski(cpuGeomTri, a, f, d, recursions - 1);
		sierpinski(cpuGeomTri, f, b, e, recursions - 1);
	}
	
};

void utmc(CPU_Geometry& cpuGeomTri, glm::vec3 a, glm::vec3 b, glm::vec3 c, int recursions)
{
	//UNIFORM TRIANGLE MASS CENTER (utmc)
	//mid points of the 3 sides of current triangle
	glm::vec3 d = glm::vec3((a.x + c.x) / 2, (a.y + c.y) / 2, 0.f);
	glm::vec3 e = glm::vec3((b.x + c.x) / 2, (b.y + c.y) / 2, 0.f);
	glm::vec3 f = glm::vec3((a.x + b.x) / 2, (a.y + b.y) / 2, 0.f);
	glm::vec3 center = glm::vec3((a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3, 0.f);
	
	if (recursions == 0) {
		cpuGeomTri.verts.push_back(a);
		cpuGeomTri.verts.push_back(b);
		cpuGeomTri.verts.push_back(c);
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
	}
	else
	{
		utmc(cpuGeomTri, center, b, c, recursions - 1);
		utmc(cpuGeomTri, a, b, center, recursions - 1);
		utmc(cpuGeomTri, a, center,c, recursions - 1);
	}

};

void koch_helper(CPU_Geometry& cpuGeomTri, glm::vec3 a, glm::vec3 e, int recursions)
{
	glm::vec3 vec_u = glm::vec3((e.x - a.x), (e.y - a.y), 0.f);
	glm::vec3 vec_v = glm::vec3(vec_u.x / 3, vec_u.y / 3, 0.f);
	double angle = atan2(vec_u.y, vec_u.x);
	angle -= M_PI / 3;
	double len_u = sqrt(pow(vec_v.x, 2) + pow(vec_v.y, 2));
	glm::vec3 b = glm::vec3((a.x + vec_v.x), (a.y + vec_v.y), 0.f);
	glm::vec3 d = glm::vec3(e.x - vec_v.x, e.y - vec_v.y, 0.f);
	glm::vec3 c = glm::vec3((b.x + (len_u * cos(angle))), (b.y + (len_u * sin(angle))), 0.f);
	
	if (recursions == 0) {
		cpuGeomTri.verts.push_back(a);
		cpuGeomTri.verts.push_back(b);
		cpuGeomTri.verts.push_back(c);
		cpuGeomTri.verts.push_back(d);
		cpuGeomTri.verts.push_back(e);
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
	}
	else {
		koch_helper(cpuGeomTri, a, b, recursions - 1);
		koch_helper(cpuGeomTri, b, c, recursions - 1);
		koch_helper(cpuGeomTri, c, d, recursions - 1);
		koch_helper(cpuGeomTri, d, e, recursions - 1);
	}
}

void koch(CPU_Geometry& cpuGeomTri, glm::vec3 a, glm::vec3 b, glm::vec3 c, int recursions)
{
	if (recursions == 0) {
		cpuGeomTri.verts.push_back(a);
		cpuGeomTri.verts.push_back(b);
		cpuGeomTri.verts.push_back(c);
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		cpuGeomTri.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
	}
	else
	{
		koch_helper(cpuGeomTri, a, b, recursions - 1);
		koch_helper(cpuGeomTri, b, c, recursions - 1);
		koch_helper(cpuGeomTri, c, a, recursions - 1);
	}

};

void dragon(CPU_Geometry& cpuGeomTri, glm::vec3 a, glm::vec3 c, int recursions, double trans, std::vector<glm::vec3>& colors, int index) {
	glm::vec3 vec_u = glm::vec3((c.x - a.x), (c.y - a.y), 0.f);
	double len_u = sqrt(pow(vec_u.x, 2) + pow(vec_u.y, 2))/sqrt(2);
	double angle = atan2(vec_u.y, vec_u.x);
	angle += trans;
	
	glm::vec3 b = glm::vec3((a.x + (len_u * cos(angle))), (a.y + (len_u * sin(angle))), 0.f);

	colors.push_back(randomColor());
	
	if (recursions == 0) {
		cpuGeomTri.verts.push_back(a);
		cpuGeomTri.verts.push_back(c);
		
		cpuGeomTri.cols.push_back(colors[index]);
		cpuGeomTri.cols.push_back(colors[index]);
		
	}
	else if (index == 0) {
		dragon(cpuGeomTri, a, b, recursions - 1, -M_PI / 4, colors, 0);
		dragon(cpuGeomTri, b, c, recursions - 1, M_PI / 4, colors, 1);
	}
	else {
		dragon(cpuGeomTri, a, b, recursions - 1, -M_PI / 4, colors, index+1);
		dragon(cpuGeomTri, b, c, recursions - 1, M_PI / 4, colors, index+1);
	}
}
int main() {
	// WINDOW
	glfwInit();
	Window window(800, 800, "FRACTALS :)"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	auto callbacks = std::make_shared<MyCallbacks>(shader);
	// CALLBACKS
	window.setCallbacks(callbacks); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeomTri;
	GPU_Geometry gpuGeomTri;

	// vertices
	State state;
	//equilateral triangle
	glm::vec3 a = glm::vec3(MIN_X, MIN_Y, 0.f);
	glm::vec3 b = glm::vec3(MAX_X, MIN_Y, 0.f);
	glm::vec3 c = glm::vec3(0.f, MAX_Y, 0.f);

	glm::vec3 a2 = glm::vec3(-0.5f, 0, 0.f);
	glm::vec3 b2 = glm::vec3(0, -0.5f, 0.f);
	glm::vec3 c2 = glm::vec3(0.5f, 0, 0.f);

	auto colors = std::vector<glm::vec3>();
	colors.push_back(randomColor());
	colors.push_back(randomColor());
	sierpinski(cpuGeomTri, a, b, c, state.recursions);
	gpuGeomTri.setVerts(cpuGeomTri.verts);
	gpuGeomTri.setCols(cpuGeomTri.cols);

	

	// RENDER LOOP
	while (!window.shouldClose()) {
		if (!(state == callbacks->getState())) {
			state = callbacks->getState();
			cpuGeomTri.verts.clear();
			cpuGeomTri.cols.clear();
			switch(state.scene) {
				case 0:
					sierpinski(cpuGeomTri, a, b, c, state.recursions);
					break;
				case 1:
					utmc(cpuGeomTri, a, b, c, state.recursions);
					break;
				case 2:
					koch(cpuGeomTri, a, b, c, state.recursions);
					break;
				case 3:
					dragon(cpuGeomTri, a2, c2, state.recursions, -M_PI / 4, colors,0);
					break;
			}
			gpuGeomTri.setVerts(cpuGeomTri.verts);
			gpuGeomTri.setCols(cpuGeomTri.cols);	
		}
		glfwPollEvents();

		shader.use();

		gpuGeomTri.bind();
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		switch (state.scene) {
		case 0:
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(cpuGeomTri.verts.size()));
			break;
		case 1:
			glDrawArrays(GL_LINE_LOOP, 0, GLsizei(cpuGeomTri.verts.size()));
			break;
		case 2:
			glDrawArrays(GL_LINE_LOOP, 0, GLsizei(cpuGeomTri.verts.size()));
			break;
		case 3:
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(cpuGeomTri.verts.size()));
			break;
		}
		
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
