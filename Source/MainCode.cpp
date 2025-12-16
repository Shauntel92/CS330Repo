#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"
#include <ranges>

// Namespace for declaring global variables
namespace
{
	// Macro for window title
	const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones"; 

	// Main GLFW window
	GLFWwindow* g_Window = nullptr;

	// scene manager object for managing the 3D scene prepare and render
	SceneManager* g_SceneManager = nullptr;
	// shader manager object for dynamic interaction with the shader code
	ShaderManager* g_ShaderManager = nullptr;
	// view manager object for managing the 3D view setup and projection to 2D
	ViewManager* g_ViewManager = nullptr;

	// --- Camera state ---
	glm::vec3 camPos = glm::vec3(0.0f, 1.2f, 6.0f);
	glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// yaw/pitch for mouse look
	float yaw = -90.0f;
	float pitch = 0.0f;

	// movement & sensitivity (scroll will adjust baseSpeed)
	float baseSpeed = 2.5f;
	float moveSpeed = 2.5f;
	float mouseSens = 0.12f;

	// frame timing
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// mouse bookkeeping
	bool   firstMouse = true;
	double lastX = 0.0, lastY = 0.0;

	// projection mode
	enum class ProjMode { Perspective, Ortho };
	ProjMode gProj = ProjMode::Perspective;

	// edge-detect flags for P/O toggles
	bool keyPWasDown = false;
	bool keyOWasDown = false;

}

// Function declarations - all functions that are called manually
// need to be pre-declared at the beginning of the source code.
bool InitializeGLFW();
bool InitializeGLEW();

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
	if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
	double xoff = xpos - lastX;
	double yoff = lastY - ypos; // invert Y
	lastX = xpos; lastY = ypos;

	xoff *= mouseSens; yoff *= mouseSens;
	yaw += (float)xoff;
	pitch += (float)yoff;
	pitch = glm::clamp(pitch, -89.0f, 89.0f);

	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camFront = glm::normalize(f);
}

void scroll_callback(GLFWwindow*, double, double yoff) {
	baseSpeed += (float)yoff * 0.25f;
	baseSpeed = glm::clamp(baseSpeed, 0.5f, 10.0f);
}

void processInput(GLFWwindow* w) {
	moveSpeed = baseSpeed * deltaTime;

	// WASD pan/zoom; QE vertical
	glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
	if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) camPos += camFront * moveSpeed;
	if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) camPos -= camFront * moveSpeed;
	if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * moveSpeed;
	if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) camPos += right * moveSpeed;
	if (glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS) camPos -= camUp * moveSpeed;
	if (glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) camPos += camUp * moveSpeed;

	// Projection toggles with edge detect
	bool pDown = glfwGetKey(w, GLFW_KEY_P) == GLFW_PRESS;
	bool oDown = glfwGetKey(w, GLFW_KEY_O) == GLFW_PRESS;
	if (pDown && !keyPWasDown) gProj = ProjMode::Perspective;
	if (oDown && !keyOWasDown) gProj = ProjMode::Ortho;
	keyPWasDown = pDown;
	keyOWasDown = oDown;
}


/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
	// if GLFW fails initialization, then terminate the application
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	// try to create a new shader manager object
	g_ShaderManager = new ShaderManager();
	// try to create a new view manager object
	g_ViewManager = new ViewManager(
		g_ShaderManager);

	// try to create the main display window
	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// Lock cursor for FPS-style look (optional)
	glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Mouse & scroll callbacks
	glfwSetCursorPosCallback(g_Window, mouse_callback);
	glfwSetScrollCallback(g_Window, scroll_callback);

	// if GLEW fails initialization, then terminate the application
	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	// load the shader code from the external GLSL files
	g_ShaderManager->LoadShaders(
		"../../Utilities/shaders/vertexShader.glsl",
		"../../Utilities/shaders/fragmentShader.glsl");
	g_ShaderManager->use();

	// try to create a new scene manager object and prepare the 3D scene
	g_SceneManager = new SceneManager(g_ShaderManager);
	g_SceneManager->PrepareScene();

	// loop will keep running until the application is closed 
	// or until an error has occurred
	while (!glfwWindowShouldClose(g_Window))
	{
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.18f, 0.12f, 0.26f, 1.0f);  // dark purple sky base
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// frame timing
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//  read keyboard (WASD/QE) and projection toggles
		processInput(g_Window);

		g_ShaderManager->use();

		// build our projection (P or O) & send to shader
		int width, height;
		glfwGetFramebufferSize(g_Window, &width, &height);
		float aspect = (height > 0) ? (float)width / (float)height : 1.0f;

		glm::mat4 projection(1.0f);
		if (gProj == ProjMode::Perspective) {
			projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
		}
		else {
			// ortho box size chosen to comfortably frame house
			float orthoH = 3.5f;
			float orthoW = orthoH * aspect;
			projection = glm::ortho(-orthoW, orthoW, -orthoH, orthoH, 0.1f, 100.0f);

			// optional: in ortho, look straight on (no horizon/floor)
			camFront = glm::vec3(0, 0, -1);
			camUp = glm::vec3(0, 1, 0);
			
		}                     
		// timing
		processInput(g_Window);

		// build projection/view
		g_ShaderManager->setMat4Value("projection", projection);

		glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
		g_ShaderManager->setMat4Value("view", view);
		g_ShaderManager->setVec3Value("viewPosition", camPos); 


		// draw the scene
		g_SceneManager->RenderScene();

		glfwSwapBuffers(g_Window);
		glfwPollEvents();
	}


	// clear the allocated manager objects from memory
	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	if (NULL != g_ShaderManager)
	{
		delete g_ShaderManager;
		g_ShaderManager = NULL;
	}

	// Terminates the program successfully
	exit(EXIT_SUCCESS); 
}

/***********************************************************
 *	InitializeGLFW()
 * 
 *  This function is used to initialize the GLFW library.   
 ***********************************************************/
bool InitializeGLFW()
{
	// GLFW: initialize and configure library
	// --------------------------------------
	glfwInit();

#ifdef __APPLE__
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	// GLFW: end -------------------------------

	return(true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
	// GLEW: initialize
	// -----------------------------------------
	GLenum GLEWInitResult = GLEW_OK;

	// try to initialize the GLEW library
	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}
	// GLEW: end -------------------------------

	// Displays a successful OpenGL initialization message
	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}