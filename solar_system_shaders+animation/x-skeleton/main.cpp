//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"
#include <math.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitCube.h"


//ANIMATION SPEED CONTROL VARIABLES
float animationSpeed = 1;
float pauseAnimation = 1;
bool restartRequested = false;
CPU_Geometry sphereTextureMapping(float rad) {
	CPU_Geometry retGeom;
	std::vector<std::vector<glm::vec3>> vertPoints;
	std::vector<std::vector<glm::vec2>> texCoorPoints;
	std::vector<std::vector<glm::vec3>> normalPoints;
	for (float u = 0; u <= 2 * M_PI; u += M_PI / 30) {
		//semiCirclePoints.push_back(glm::vec3(cos(radians_i), sin(radians_i), 1.f));

		vertPoints.push_back(std::vector<glm::vec3>());
		texCoorPoints.push_back(std::vector<glm::vec2>());
		normalPoints.push_back(std::vector<glm::vec3>());
		for (float v = 0; v <= M_PI; v += M_PI / 30) {
			float x = rad * sin(v) * cos(u);
			float y = rad * cos(v);

			float z = rad * sin(v) * sin(u);

			vertPoints.back().push_back(glm::vec3(x, y, z));
			texCoorPoints.back().push_back(glm::vec2(u / (2 * M_PI), v / (M_PI)));
			float mag = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
			normalPoints.back().push_back(glm::vec3(x / mag, y / mag, z / mag));

		}
	}
	for (float j = 0; j < vertPoints.size(); j++) {

		for (float k = 0; k < vertPoints[0].size() - 1; k++)
		{
			//std::cout << "Size " << vertPoints[2].size() << " size " << vertPoints.size() << std::endl;
			//std::cout << "j, k " << j << " " << k << std::endl;
			if (j == vertPoints.size() - 1)
			{
				retGeom.verts.push_back(vertPoints[0][k]);
				retGeom.verts.push_back(vertPoints[j][k]);
				retGeom.verts.push_back(vertPoints[j][k + 1]);

				retGeom.texCoords.push_back(texCoorPoints[0][k]);
				retGeom.texCoords.push_back(texCoorPoints[j][k]);
				retGeom.texCoords.push_back(texCoorPoints[j][k + 1]);


				retGeom.normals.push_back(normalPoints[0][k]);
				retGeom.normals.push_back(normalPoints[j][k]);
				retGeom.normals.push_back(normalPoints[j][k + 1]);

				retGeom.verts.push_back(vertPoints[j][k + 1]);
				retGeom.verts.push_back(vertPoints[0][k + 1]);
				retGeom.verts.push_back(vertPoints[0][k]);


				retGeom.texCoords.push_back(texCoorPoints[j][k + 1]);
				retGeom.texCoords.push_back(texCoorPoints[0][k + 1]);
				retGeom.texCoords.push_back(texCoorPoints[0][k]);

				retGeom.normals.push_back(normalPoints[j][k + 1]);
				retGeom.normals.push_back(normalPoints[0][k+1]);
				retGeom.normals.push_back(normalPoints[0][k]);
			}
			else {
				retGeom.verts.push_back(vertPoints[j + 1][k]);
				retGeom.verts.push_back(vertPoints[j][k]);
				retGeom.verts.push_back(vertPoints[j][k + 1]);

				retGeom.texCoords.push_back(texCoorPoints[j + 1][k]);
				retGeom.texCoords.push_back(texCoorPoints[j][k]);
				retGeom.texCoords.push_back(texCoorPoints[j][k + 1]);

				retGeom.normals.push_back(normalPoints[j + 1][k]);
				retGeom.normals.push_back(normalPoints[j][k]);
				retGeom.normals.push_back(normalPoints[j][k + 1]);


				retGeom.verts.push_back(vertPoints[j][k + 1]);
				retGeom.verts.push_back(vertPoints[j + 1][k + 1]);
				retGeom.verts.push_back(vertPoints[j + 1][k]);


				retGeom.texCoords.push_back(texCoorPoints[j][k + 1]);
				retGeom.texCoords.push_back(texCoorPoints[j + 1][k + 1]);
				retGeom.texCoords.push_back(texCoorPoints[j + 1][k]);

				retGeom.normals.push_back(normalPoints[j][k + 1]);
				retGeom.normals.push_back(normalPoints[j+1][k + 1]);
				retGeom.normals.push_back(normalPoints[j+1][k]);
			}


		}
	}
	return retGeom;
}
struct CelestialObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	CelestialObject(std::string texturePath, GLenum textureInterpolation, float rad) :
		texture(texturePath, textureInterpolation),
		//transformationMatrix(1),
		parent(nullptr),
		radius(rad),
		translationMatrix(1),
		axisTilt(0),
		orbitTilt(0),
		orbitRadius(0),
		orbitProgress(0),
		axisProgress(0),
		ambient(0.2f)

	{
		//time dependant
		float orbitProgress;
		float axisProgress;
		cgeom = sphereTextureMapping(radius);
		ggeom.setTexCoords(cgeom.texCoords);
		ggeom.setVerts(cgeom.verts);
		ggeom.setNormals(cgeom.normals);
	}
	
	
	//update and return transformation matrix before it is set to vertex shader 
	glm::mat4 updateTransformationMatrix() {
		glm::mat4 transformationMatrix = glm::rotate(axisProgress, glm::vec3(0, 1, 0)) * glm::mat4(1); //update rotation for all 
		if (parent != nullptr)
		{
			transformationMatrix = glm::rotate(axisTilt, glm::vec3(0, 0, 1)) *transformationMatrix; 
			glm::vec3 calc = glm::vec3(orbitRadius, 0, 0);
			calc = glm::rotate(orbitTilt, glm::vec3(0, 0, 1)) * glm::rotate(orbitProgress, glm::vec3(0, 1, 0)) * glm::vec4(calc,1); //revolution for earth/ moon
			translationMatrix = parent->translationMatrix * glm::translate(glm::mat4(1.0), calc);
			return translationMatrix * transformationMatrix;
		}
		else {
			return transformationMatrix;
		}
		
	}
	void orbitAxisCalc(float time)
	{
		float rotation = 0.9;
		float revolution = 0.1;
		axisProgress = fmod(axisProgress + rotation * time * M_PI, 2 * M_PI);
		orbitProgress = fmod(orbitProgress + revolution * time * M_PI, 2 * M_PI);
	}
	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;
	glm::mat4 translationMatrix;
	struct CelestialObject* parent;

	//shading coeff	
	float specular;
	float diffuse;
	float ambient;

	//unchanging everpresent
	float radius;
	float orbitRadius;
	float axisTilt;
	float orbitTilt;

	//time dependant
	float orbitProgress;
	float axisProgress;

};
// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 32.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		{
			pauseAnimation = pauseAnimation == 1 ? 0 : 1;
		}
		else if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && action == GLFW_RELEASE)
		{
			if (animationSpeed < 8) {
				animationSpeed *= 1.5;
				pauseAnimation = 1;
			}
		}
		else if ((key== GLFW_KEY_A ||key == GLFW_KEY_LEFT) && action == GLFW_RELEASE)
		{
			if (animationSpeed > 0.25) {
				animationSpeed /= 1.5;
				pauseAnimation = 1;
			}
			
		}
		else if ((key == GLFW_KEY_R) && action == GLFW_RELEASE)
		{
			restartRequested = !restartRequested;
			animationSpeed = 1;
			pauseAnimation = 1;
		}

	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}
	Camera camera;
private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

auto a4 = std::make_shared<Assignment4>();

void setModelSpace(ShaderProgram& shader, CelestialObject& obj)
{
	glUniformMatrix4fv(glGetUniformLocation(shader.getProgram(), "M"), 1, GL_FALSE, glm::value_ptr(obj.updateTransformationMatrix()));
	glUniform1f(glGetUniformLocation(shader.getProgram(), "specular"), obj.specular);
	glUniform1f(glGetUniformLocation(shader.getProgram(), "diffuse"), obj.diffuse);
	glUniform1f(glGetUniformLocation(shader.getProgram(), "ambient"), obj.ambient);
	glUniform3fv(glGetUniformLocation(shader.getProgram(), "cameraPos"), 1, glm::value_ptr(a4->camera.getPos()));
	obj.ggeom.bind();
	obj.texture.bind();

	glDrawArrays(GL_TRIANGLES, 0, GLsizei(obj.cgeom.verts.size()));
	obj.texture.unbind();
}
void resetAnimation(CelestialObject& obj, CelestialObject& obj1, CelestialObject& obj2)
{
	obj.axisProgress = 0;
	obj1.axisProgress = 0;
	obj2.axisProgress = 0;

	obj.orbitProgress = 0;
	obj1.orbitProgress = 0;
	obj2.orbitProgress = 0;
}
int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "Solar System");

	GLDebug::enable();

	// CALLBACKS
	
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");


	CelestialObject stars = CelestialObject("textures/stars.jpg", GL_NEAREST, 150);
	stars.diffuse = 0.3f;
	stars.specular = 0.2f;
	CelestialObject sun = CelestialObject("textures/sun.jpg", GL_NEAREST, 4);
	//sun.createRotMat();
	sun.ambient = 0.9f;

	CelestialObject earth = CelestialObject("textures/earth.jpg", GL_NEAREST,2);
	//earth.createRotMat();
	earth.axisTilt = 203.45*M_PI/180;
	earth.ambient = 0.1;
	earth.orbitRadius = 25;
	earth.orbitTilt = M_PI/10;
	earth.parent = &sun;
	earth.diffuse = 0.5f;
	earth.specular = 0.3f;
	

	CelestialObject moon = CelestialObject("textures/moon.jpg", GL_NEAREST, 1);
	//moon.createRotMat();
	moon.axisTilt = 203.45 * M_PI / 180;
	moon.orbitRadius = 5;
	moon.orbitTilt = 75 * M_PI / 180;
	moon.parent = &earth;
	moon.diffuse = 0.8f;
	moon.specular = 0.5f;
	
	double currentTime = 0;
	double prevTime = 0;
	// RENDER LOOP
	while (!window.shouldClose()) {
		currentTime = glfwGetTime();
		if (restartRequested)
		{
			resetAnimation(sun,earth,moon);
			restartRequested = false;
		}
		sun.orbitAxisCalc(animationSpeed * pauseAnimation * 0.1 * float((currentTime - prevTime)));
		earth.orbitAxisCalc(animationSpeed * pauseAnimation *  0.3 * float((currentTime - prevTime)));
		moon.orbitAxisCalc(animationSpeed * pauseAnimation *  0.5 * float((currentTime - prevTime)));
		glfwPollEvents();
		//glEnable(GL_TEXTURE_2D);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		shader.use();

		a4->viewPipeline(shader);
	
		setModelSpace(shader, stars);
		
		setModelSpace(shader, sun);
		
		setModelSpace(shader, earth);
		
		setModelSpace(shader, moon);

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();

		prevTime = currentTime;
	}
	glfwTerminate();
	return 0;
}
