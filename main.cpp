#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>

#include "Headers/TimeManager.h"

#include "Headers/Shader.h"

#include "Headers/Sphere.h"
#include "Headers/Cylinder.h"
#include "Headers/Box.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/ThirdPersonCamera.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/Texture.h"

#include "Headers/Model.h"

#include "Headers/Terrain.h"

#include "Headers/AnimationUtils.h"

#include "Headers/Colisiones.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

int screenWidth;
int screenHeight;

GLFWwindow *window;

Shader shader;
Shader shaderSkybox;
Shader shaderMulLighting;
Shader shaderTerrain;

std::shared_ptr<Camera> camera(new ThirdPersonCamera());
float distanceFromTarget = 15.0;

Sphere skyboxSphere(20, 20);
Box boxCollider;
Sphere sphereCollider(10, 10);

// Pulpo
Model pulpoModel;
Model pulpoModel2;

// Tiburones
Model tiburonModel;
Model tiburonModel2;
Model tiburonModel3;
Model tiburonModel4;
Model tiburonModel5;
Model tiburonModel6;
Model tiburonModel7;
Model tiburonModel8;

Terrain terrain(-1, -1, 200, 8, "../texturasProyFinal/CustomHeight4.png");

GLuint textureCespedID, textureWallID, textureWindowID, textureHighwayID, textureLandingPadID;
GLuint textureTerrainBackgroundID, textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint skyboxTextureID;

GLenum types[6] = {
GL_TEXTURE_CUBE_MAP_POSITIVE_X,
GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

std::string fileNames[6] = { "../texturasProyFinal/Simple.jpg",
		"../texturasProyFinal/Simple.jpg",
		"../texturasProyFinal/Simple.jpg",
		"../texturasProyFinal/Simple.jpg",
		"../texturasProyFinal/Simple.jpg",
		"../texturasProyFinal/Simple.jpg" };

bool exitApp = false;
int lastMousePosX, offsetX = 0;
int lastMousePosY, offsetY = 0;

glm::mat4 modelMatrixPulpo = glm::mat4(1.0f);
glm::mat4 modelMatrixPulpo2 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon2 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon3 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon4 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon5 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon6 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon7 = glm::mat4(1.0f);
glm::mat4 modelMatrixTiburon8 = glm::mat4(1.0f);

int animationIndex = 1;
double deltaTime;
double currTime, lastTime;
int state = 0;

bool isJump = false;
float GRAVITY = 0.2;
double tmv = 0;
double startTimeJump = 0;

std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > collidersOBB;
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > collidersSBB;

void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
bool processInput(bool continueApplication = true);

void init(int width, int height, std::string strTitle, bool bFullScreen) {

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		exit(-1);
	}

	screenWidth = width;
	screenHeight = height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (bFullScreen)
		window = glfwCreateWindow(width, height, strTitle.c_str(),
			glfwGetPrimaryMonitor(), nullptr);
	else
		window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr,
			nullptr);

	if (window == nullptr) {
		std::cerr
				<< "Error to create GLFW window, you can try download the last version of your video card that support OpenGL 3.3+"
				<< std::endl;
		destroy();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > collidersOBB;
	std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > collidersSBB;

	glfwSetWindowSizeCallback(window, reshapeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Failed to initialize glew" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	shader.initialize("../Shaders/colorShader.vs", "../Shaders/colorShader.fs");
	shaderSkybox.initialize("../Shaders/skyBox.vs", "../Shaders/skyBox.fs");
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation_fog.vs", "../Shaders/multipleLights_fog.fs");
	shaderTerrain.initialize("../Shaders/terrain_fog.vs", "../Shaders/terrain_fog.fs");

	skyboxSphere.init();
	skyboxSphere.setShader(&shaderSkybox);
	skyboxSphere.setScale(glm::vec3(20.0f, 20.0f, 20.0f));

	boxCollider.init();
	boxCollider.setShader(&shader);
	boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	sphereCollider.init();
	sphereCollider.setShader(&shader);
	sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	terrain.init();
	terrain.setShader(&shaderTerrain);
	terrain.setPosition(glm::vec3(100, 0, 100));

	//Pulpo
	pulpoModel.loadModel("../texturasProyFinal/Calamar.obj");
	pulpoModel.setShader(&shaderMulLighting);

	pulpoModel2.loadModel("../texturasProyFinal/Calamar.obj");
	pulpoModel2.setShader(&shaderMulLighting);

	//Tiburon
	tiburonModel.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel.setShader(&shaderMulLighting);

	tiburonModel2.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel2.setShader(&shaderMulLighting);

	tiburonModel3.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel3.setShader(&shaderMulLighting);

	tiburonModel4.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel4.setShader(&shaderMulLighting);

	tiburonModel5.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel5.setShader(&shaderMulLighting);

	tiburonModel6.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel6.setShader(&shaderMulLighting);

	tiburonModel7.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel7.setShader(&shaderMulLighting);

	tiburonModel8.loadModel("../texturasProyFinal/Tiburon.fbx");
	tiburonModel8.setShader(&shaderMulLighting);

	camera->setPosition(glm::vec3(0.0, 0.0, 10.0));
	camera->setDistanceFromTarget(distanceFromTarget);
	camera->setSensitivity(1.0);

	int imageWidth, imageHeight;
	FIBITMAP *bitmap;
	unsigned char *data;

	Texture skyboxTexture = Texture("");
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
		skyboxTexture = Texture(fileNames[i]);
		FIBITMAP *bitmap = skyboxTexture.loadImage(true);
		unsigned char *data = skyboxTexture.convertToData(bitmap, imageWidth,
				imageHeight);
		if (data) {
			glTexImage2D(types[i], 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		} else
			std::cout << "Failed to load texture" << std::endl;
		skyboxTexture.freeImage(bitmap);
	}

	Texture textureTerrainBackground("../texturasProyFinal/Sand.jpg");
	bitmap = textureTerrainBackground.loadImage();
	data = textureTerrainBackground.convertToData(bitmap, imageWidth,
			imageHeight);
	glGenTextures(1, &textureTerrainBackgroundID);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBackgroundID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else
		std::cout << "Failed to load texture" << std::endl;
	textureTerrainBackground.freeImage(bitmap);

	Texture textureTerrainR("../texturasProyFinal/Sand2.jpg");
	bitmap = textureTerrainR.loadImage();
	data = textureTerrainR.convertToData(bitmap, imageWidth,
			imageHeight);
	glGenTextures(1, &textureTerrainRID);
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else
		std::cout << "Failed to load texture" << std::endl;
	textureTerrainR.freeImage(bitmap);

	Texture textureTerrainB("../texturasProyFinal/Sand3.jpg");
	bitmap = textureTerrainB.loadImage();
	data = textureTerrainB.convertToData(bitmap, imageWidth,
			imageHeight);
	glGenTextures(1, &textureTerrainBID);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else
		std::cout << "Failed to load texture" << std::endl;
	textureTerrainB.freeImage(bitmap);

	Texture textureTerrainBlendMap("../texturasProyFinal/blendMap3.png");
	bitmap = textureTerrainBlendMap.loadImage(true);
	data = textureTerrainBlendMap.convertToData(bitmap, imageWidth,
			imageHeight);
	glGenTextures(1, &textureTerrainBlendMapID);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else
		std::cout << "Failed to load texture" << std::endl;
	textureTerrainBlendMap.freeImage(bitmap);
}

void destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();
	shader.destroy();
	shaderMulLighting.destroy();
	shaderSkybox.destroy();
	shaderTerrain.destroy();
	skyboxSphere.destroy();
	boxCollider.destroy();
	sphereCollider.destroy();
	terrain.destroy();

	pulpoModel.destroy();
	pulpoModel2.destroy();
	tiburonModel.destroy();
	tiburonModel2.destroy();
	tiburonModel3.destroy();
	tiburonModel4.destroy();
	tiburonModel5.destroy();
	tiburonModel6.destroy();
	tiburonModel7.destroy();
	tiburonModel8.destroy();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &textureCespedID);
	glDeleteTextures(1, &textureTerrainBackgroundID);
	glDeleteTextures(1, &textureTerrainRID);
	glDeleteTextures(1, &textureTerrainGID);
	glDeleteTextures(1, &textureTerrainBID);
	glDeleteTextures(1, &textureTerrainBlendMapID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &skyboxTextureID);
}

void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes) {
	screenWidth = widthRes;
	screenHeight = heightRes;
	glViewport(0, 0, widthRes, heightRes);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			exitApp = true;
			break;
		}
	}
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
	offsetX = xpos - lastMousePosX;
	offsetY = ypos - lastMousePosY;
	lastMousePosX = xpos;
	lastMousePosY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	distanceFromTarget -= yoffset;
	camera->setDistanceFromTarget(distanceFromTarget);
}

void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod) {
	if (state == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		}
	}
}

bool processInput(bool continueApplication) {
	if (exitApp || glfwWindowShouldClose(window) != 0) {
		return false;
	}

	//-----------------------Basicos--------------------
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		modelMatrixPulpo = glm::translate(modelMatrixPulpo, glm::vec3(0, 0, 0.1));
	}else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		modelMatrixPulpo = glm::translate(modelMatrixPulpo, glm::vec3(0, 0, -0.1));
	}else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		modelMatrixPulpo = glm::translate(modelMatrixPulpo, glm::vec3(0.1, 0, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		modelMatrixPulpo = glm::translate(modelMatrixPulpo, glm::vec3(-0.1, 0, 0));
	}
	//---------------------Salto--------------------------
	bool keySpaceStatus = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	if (keySpaceStatus) {
		tmv = 0;
		startTimeJump = currTime;
	}

	glfwPollEvents();
	return continueApplication;
}

void applicationLoop() {
	bool psi = true;
	int numberAdvance = 0;
	int maxAdvance = 30.0;
	float advanceCount = 0.0;
	glm::mat4 view;
	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

	modelMatrixPulpo = glm::translate(modelMatrixPulpo, glm::vec3(15.0f, 10.0f, -90.0f));
	modelMatrixPulpo = glm::scale(modelMatrixPulpo, glm::vec3(0.5f, 0.5f, 0.5f));

	modelMatrixPulpo2 = glm::translate(modelMatrixPulpo2, glm::vec3(-15.0f, 10.0f, 90.0f));
	modelMatrixPulpo2 = glm::scale(modelMatrixPulpo2, glm::vec3(0.5f, 0.5f, 0.5f));

	//----------------------Tiburones--------------------------------

	modelMatrixTiburon = glm::translate(modelMatrixTiburon, glm::vec3(-20.0f, 3.0f, -70.0f));
	modelMatrixTiburon = glm::rotate(modelMatrixTiburon, glm::radians(-90.0f), glm::vec3(0, 1, 0));

	modelMatrixTiburon2 = glm::translate(modelMatrixTiburon2, glm::vec3(-20.0f, 7.5f, -70.0f));
	modelMatrixTiburon2 = glm::rotate(modelMatrixTiburon2, glm::radians(-90.0f), glm::vec3(0, 1, 0));

	lastTime = TimeManager::Instance().GetTime();

	while (psi) {
		currTime = TimeManager::Instance().GetTime();
		if (currTime - lastTime < 0.016666667) {
			glfwPollEvents();
			continue;
		}
		lastTime = currTime;
		TimeManager::Instance().CalculateFrameRate(true);
		deltaTime = TimeManager::Instance().DeltaTime;
		psi = processInput(true);

		std::map<std::string, bool> collisionDetection;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f),
			(float)screenWidth / (float)screenHeight, 0.01f, 100.0f);

		axis = glm::axis(glm::quat_cast(modelMatrixPulpo));
		angleTarget = glm::angle(glm::quat_cast(modelMatrixPulpo));
		target = modelMatrixPulpo[3];

		if (std::isnan(angleTarget))
			angleTarget = 0.0;
		if (axis.y < 0)
			angleTarget = -angleTarget;
		angleTarget -= glm::radians(90.0f);
		camera->setCameraTarget(target);
		camera->setAngleTarget(angleTarget);
		camera->updateCamera();
		view = camera->getViewMatrix();

		shader.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shader.setMatrix4("view", 1, false, glm::value_ptr(view));

		shaderSkybox.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderSkybox.setMatrix4("view", 1, false,
			glm::value_ptr(glm::mat4(glm::mat3(view))));
		shaderMulLighting.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderMulLighting.setMatrix4("view", 1, false,
			glm::value_ptr(view));
		shaderTerrain.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderTerrain.setMatrix4("view", 1, false,
			glm::value_ptr(view));

		//-------------------------Neblina-----------------------
		shaderMulLighting.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.1, 0.1, 0.9)));
		shaderTerrain.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.0, 0.0, 0.9)));

		/*******************************************
		 * Propiedades Luz direccional
		 *******************************************/
		shaderMulLighting.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

		/*******************************************
		 * Propiedades Luz direccional Terrain
		 *******************************************/
		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

		/*******************************************
		 * Terrain Cesped
		 *******************************************/
		glm::mat4 modelCesped = glm::mat4(1.0);
		modelCesped = glm::translate(modelCesped, glm::vec3(0.0, 0.0, 0.0));
		modelCesped = glm::scale(modelCesped, glm::vec3(200.0, 0.001, 200.0));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureTerrainBackgroundID);
		shaderTerrain.setInt("backgroundTexture", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
		shaderTerrain.setInt("rTexture", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureTerrainGID);
		shaderTerrain.setInt("gTexture", 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureTerrainBID);
		shaderTerrain.setInt("bTexture", 3);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID);
		shaderTerrain.setInt("blendMapTexture", 4);
		shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(40, 40)));
		terrain.render();
		shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(0, 0)));
		glBindTexture(GL_TEXTURE_2D, 0);

		/*******************************************
		 * Custom Anim objects obj
		 *******************************************/

		//------------------- Pulpo ----------------------------------------

		modelMatrixPulpo[3][1] = -GRAVITY * tmv * tmv + 0.3 * tmv + modelMatrixPulpo[3][1];
		tmv = currTime - startTimeJump;
		if (modelMatrixPulpo[3][1] < terrain.getHeightTerrain(modelMatrixPulpo[3][0], modelMatrixPulpo[3][2])) {
			modelMatrixPulpo[3][1] = terrain.getHeightTerrain(modelMatrixPulpo[3][0], modelMatrixPulpo[3][2]);
		}
		glm::mat4 modelMatrixPulpoBody = glm::mat4(modelMatrixPulpo);
		pulpoModel.setAnimationIndex(animationIndex);
		pulpoModel.render(modelMatrixPulpoBody);

		glm::mat4 modelMatrixPulpo2Body = glm::mat4(modelMatrixPulpo2);
		pulpoModel2.setAnimationIndex(animationIndex);
		pulpoModel2.render(modelMatrixPulpo2Body);

		//------------------- Tiburones ----------------------------------------
		glm::mat4 modelMatrixTiburonBody = glm::mat4(modelMatrixTiburon);
		tiburonModel.setAnimationIndex(3);
		tiburonModel.render(modelMatrixTiburonBody);

		glm::mat4 modelMatrixTiburonBody2 = glm::mat4(modelMatrixTiburon2);
		tiburonModel2.setAnimationIndex(3);
		tiburonModel2.render(modelMatrixTiburonBody2);


		/*******************************************
		 * Skybox
		 *******************************************/
		GLint oldCullFaceMode;
		GLint oldDepthFuncMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
		shaderSkybox.setFloat("skybox", 0);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		skyboxSphere.render();
		glCullFace(oldCullFaceMode);
		glDepthFunc(oldDepthFuncMode);

		/*******************************************
		 * Creacion de colliders
		 * IMPORTANT do this before interpolations
		 *******************************************/

		 //Collider del pulpo
		AbstractModel::SBB pulpoCollider;
		glm::mat4 modelMatrixColliderPulpo = glm::mat4(modelMatrixPulpo);
		modelMatrixColliderPulpo = glm::scale(modelMatrixColliderPulpo, glm::vec3(0.2, 0.2, 0.2));
		modelMatrixColliderPulpo = glm::translate(modelMatrixColliderPulpo, pulpoModel.getSbb().c);
		pulpoCollider.c = glm::vec3(modelMatrixColliderPulpo[3]);
		pulpoCollider.ratio = pulpoModel.getSbb().ratio * 1.0;
		addOrUpdateColliders(collidersSBB, "pulpo", pulpoCollider, modelMatrixPulpo);

		//Collider del Tiburon
		AbstractModel::SBB tiburonCollider;
		glm::mat4 modelMatrixColliderTiburon = glm::mat4(modelMatrixTiburon);
		modelMatrixColliderTiburon = glm::scale(modelMatrixColliderTiburon, glm::vec3(0.2, 0.2, 0.2));
		modelMatrixColliderTiburon = glm::translate(modelMatrixColliderTiburon, tiburonModel.getSbb().c);
		tiburonCollider.c = glm::vec3(modelMatrixColliderTiburon[3]);
		tiburonCollider.ratio = tiburonModel.getSbb().ratio * 1.0;
		addOrUpdateColliders(collidersSBB, "tiburon", tiburonCollider, modelMatrixTiburon);

		/*******************************************
		 * Render de colliders
		 *******************************************/
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it =
				collidersOBB.begin(); it != collidersOBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = matrixCollider * glm::mat4(std::get<0>(it->second).u);
			matrixCollider = glm::scale(matrixCollider, std::get<0>(it->second).e * 2.0f);
			boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			boxCollider.enableWireMode();
			boxCollider.render(matrixCollider);
		}

		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it =
				collidersSBB.begin(); it != collidersSBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(it->second).ratio * 2.0f));
			sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			sphereCollider.enableWireMode();
			sphereCollider.render(matrixCollider);
		}

		//----------------Prueba de Colisiones------------------
		for (std::map<std::string, 
			std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator it =
			collidersOBB.begin(); it != collidersOBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string, 
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersOBB.begin(); jt != collidersOBB.end() && !isCollision; jt++) {
				if (it != jt && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					std::cout << "Collision " << it->first << " with " << jt->first << std::endl;
					isCollision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		for (std::map<std::string,
			std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string,
				std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersSBB.begin(); jt != collidersSBB.end() && !isCollision; jt++) {
				if (it != jt && testSphereSphereIntersection(std::get<0>(it->second), std::get<0>(jt->second))) {
					std::cout << "Collision " << it->first << " with " << jt->first << std::endl;
					isCollision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		for (std::map<std::string,
			std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersOBB.begin(); jt != collidersOBB.end() && !isCollision; jt++) {
				if (testSphereOBox(std::get<0>(it->second), std::get<0>(jt->second))) {
					std::cout << "Collision " << it->first << " with " << jt->first << std::endl;
					isCollision = true;
					addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		std::map<std::string, bool>::iterator colIt;
		for (colIt = collisionDetection.begin(); colIt != collisionDetection.end(); colIt++) {
			std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it = collidersOBB.find(colIt->first);
			std::map<std::string,
				std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator jt = collidersSBB.find(colIt->first);
			if (it != collidersOBB.end()) {
				if (!colIt->second)
					addOrUpdateColliders(collidersOBB, it->first);
				else {
					if (it->first.compare("pulpo") == 0)
						modelMatrixPulpo = std::get<1>(it->second);
				}
			}
			if (jt != collidersSBB.end()) {
				if (!colIt->second)
					addOrUpdateColliders(collidersSBB, jt->first);
			}
		}

		//------------ Maquina de estado Tiburon -------------------
		switch (state) {
		case 0:
			modelMatrixTiburon = glm::translate(modelMatrixTiburon, glm::vec3(0.0, 0.0, -0.2));
			modelMatrixTiburon2 = glm::translate(modelMatrixTiburon2, glm::vec3(0.0, 0.0, -0.2));
			advanceCount += 0.1;
			if (advanceCount >= maxAdvance) {
				advanceCount = 0;
				state = 1;
			}
			break;

		case 1:
			modelMatrixTiburon = glm::rotate(modelMatrixTiburon, glm::radians(180.0f), glm::vec3(0, 1, 0));
			modelMatrixTiburon2 = glm::rotate(modelMatrixTiburon2, glm::radians(180.0f), glm::vec3(0, 1, 0));
			state = 2;
			break;

		case 2:
			modelMatrixTiburon = glm::translate(modelMatrixTiburon, glm::vec3(0.0, 0.0, -0.2));
			modelMatrixTiburon2 = glm::translate(modelMatrixTiburon2, glm::vec3(0.0, 0.0, -0.2));
			advanceCount += 0.1;
			if (advanceCount >= maxAdvance) {
				advanceCount = 0;
				state = 3;
			}
			break;

		case 3:
			modelMatrixTiburon = glm::rotate(modelMatrixTiburon, glm::radians(180.0f), glm::vec3(0, 1, 0));
			modelMatrixTiburon2 = glm::rotate(modelMatrixTiburon2, glm::radians(180.0f), glm::vec3(0, 1, 0));
			state = 0;
			break;
		}

		glfwSwapBuffers(window);
	}
}

int main(int argc, char **argv) {
	init(1600, 900, "Window GLFW", false);
	applicationLoop();
	destroy();
	return 1;
}
