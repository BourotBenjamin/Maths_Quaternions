// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Quaternion.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <soil/SOIL.h>
#define M_PI 3.14159265359

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
float pointLightCoeff = 0.0f, dirLightCoeff = 1.0f;
glm::mat4 rotationMatrix;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;



// The MAIN function, from here we start our application and run our Game loop
int main(int argc, char* argv[])
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile our shaders
	Shader shader("basic.vs", "basic.fs");
	//Shader lightShader("light.vs", "light.fs");


	// Load models
	Model ourModel("./models/nano/nanosuit.obj");


	// Draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		// Clear the colorbuffer
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();   // <-- Don't forget this one!
		// Transformation matrices
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix() * rotationMatrix;
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Draw the loaded model
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		GLint viewPosLoc = glGetUniformLocation(shader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		/********* DIR LIGHT *********/
		glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.0f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.0f, 0.4f, 0.4f);
		glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.0f, 0.05f, 0.05f);
		glUniform1f(glGetUniformLocation(shader.Program, "dirLight.coeff"), dirLightCoeff);


		/********* POINT LIGHTS *********/
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), 1.0f + sin(glfwGetTime()) * 2.0f, sin(glfwGetTime() / 2.0f) * 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.05f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 0.8f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 0.1f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].coeff"), pointLightCoeff);

		ourModel.Draw(shader);

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_R])
		rotationMatrix = glm::mat4();
	if (keys[GLFW_KEY_I])
		dirLightCoeff = 1.0f;
	if (keys[GLFW_KEY_K])
		dirLightCoeff = 0.0f;
	if (keys[GLFW_KEY_O])
		pointLightCoeff = 1.0f;
	if (keys[GLFW_KEY_L])
		pointLightCoeff = 0.0f;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;


	Quaternion qY(
		0,
		sin(((xoffset)* M_PI) / screenWidth),
		0,
		cos(((xoffset)* M_PI) / screenWidth)
		);
	Quaternion qX(
		sin(((yoffset)* M_PI) / screenHeight),
		0,
		0,
		cos(((yoffset)* M_PI) / screenHeight)
		);
	rotationMatrix *= Quaternion::multiply(qY, qX).toMatrixUnit();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion