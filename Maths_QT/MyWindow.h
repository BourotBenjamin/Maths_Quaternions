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
glm::vec3 lightPos;


void RenderScene(Shader& shader, Model& ourModel)
{

	shader.Use();   // <-- Don't forget this one!
	// Transformation matrices
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
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
	glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.05f, 0.05f, 0.02f);
	glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.4f, 0.4f, 0.2f);
	glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.05f, 0.05f, 0.02f);
	glUniform1f(glGetUniformLocation(shader.Program, "dirLight.coeff"), dirLightCoeff);


	/********* POINT LIGHTS *********/
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 0.1f, 0.1f, 0.1f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032);
	glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].coeff"), pointLightCoeff);

	ourModel.Draw(shader);

}




// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-10.0f, 10.0f, -5.0f, 0.0f, 1024.0f,
			-10.0f, -10.0f, -5.0f, 0.0f, 0.0f,
			10.0f, 10.0f, -5.0f, 1024.0f, 1024.0f,
			10.0f, -10.0f, -5.0f, 1024.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

GLuint loadTexture(GLchar* path)
{
	// Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;

}

GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}



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
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Setup and compile our shaders
	Shader shader("basic.vs", "basic.fs");
	Shader depthShader("depth.vs", "depth.fs");
	Shader shaderDebug("debug.vs", "debug.fs");
	Shader skyShader("sky.vs", "sky.fs");


	// Load models
	Model nano("./models/nano/nanosuit.obj");
	Model ground("./models/nano/nano.obj");

	GLint groundTexture = loadTexture("./models/nano/groung.png");


	// Draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


#pragma region "object_initialization"
	GLfloat skyboxVertices[] = {
		// Positions          
		-30.0f, 30.0f, -30.0f,
		-30.0f, -30.0f, -30.0f,
		30.0f, -30.0f, -30.0f,
		30.0f, -30.0f, -30.0f,
		30.0f, 30.0f, -30.0f,
		-30.0f, 30.0f, -30.0f,

		-30.0f, -30.0f, 30.0f,
		-30.0f, -30.0f, -30.0f,
		-30.0f, 30.0f, -30.0f,
		-30.0f, 30.0f, -30.0f,
		-30.0f, 30.0f, 30.0f,
		-30.0f, -30.0f, 30.0f,

		30.0f, -30.0f, -30.0f,
		30.0f, -30.0f, 30.0f,
		30.0f, 30.0f, 30.0f,
		30.0f, 30.0f, 30.0f,
		30.0f, 30.0f, -30.0f,
		30.0f, -30.0f, -30.0f,

		-30.0f, -30.0f, 30.0f,
		-30.0f, 30.0f, 30.0f,
		30.0f, 30.0f, 30.0f,
		30.0f, 30.0f, 30.0f,
		30.0f, -30.0f, 30.0f,
		-30.0f, -30.0f, 30.0f,

		-30.0f, 30.0f, -30.0f,
		30.0f, 30.0f, -30.0f,
		30.0f, 30.0f, 30.0f,
		30.0f, 30.0f, 30.0f,
		-30.0f, 30.0f, 30.0f,
		-30.0f, 30.0f, -30.0f,

		-30.0f, -30.0f, -30.0f,
		-30.0f, -30.0f, 30.0f,
		30.0f, -30.0f, -30.0f,
		30.0f, -30.0f, -30.0f,
		-30.0f, -30.0f, 30.0f,
		30.0f, -30.0f, 30.0f
	};
	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

#pragma endregion

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height;
	unsigned char* image;
	vector<const GLchar*> faces;
	faces.push_back("./models/skybox/front.png");
	faces.push_back("./models/skybox/back.png");
	faces.push_back("./models/skybox/top.png");
	faces.push_back("./models/skybox/bottom.png");
	faces.push_back("./models/skybox/right.png");
	faces.push_back("./models/skybox/left.png");
	GLuint cubemapTexture = loadCubemap(faces);


	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		lightPos = glm::vec3(1.0f + sin(glfwGetTime()) * 2.0f, sin(glfwGetTime() / 2.0f) * 1.0f, 2.0f);
		lightPos = glm::vec3(2.0f, 0.5f, 1.0f);
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();


		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 50.0f);
		glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 lightSpaceMatrix = depthProjectionMatrix * depthViewMatrix;

		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));


		// 1. first render to depth map
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(depthShader, nano);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// 2. then render scene as normal with shadow mapping (using depth map)
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(shader.Program, "shadowMap"), 0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(shader, nano);
		RenderScene(shader, ground);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		/*
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(skyShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glDepthMask(GL_FALSE);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		*/
		// Draw the loaded model
		shaderDebug.Use();
		glUniformMatrix4fv(glGetUniformLocation(shaderDebug.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shaderDebug.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shaderDebug.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderQuad(); // uncomment this line to see depth map
		/*
		shaderDebug.Use();
		glUniformMatrix4fv(glGetUniformLocation(shaderDebug.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(shaderDebug, nano);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*/
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

	camera.ProcessMouseMovement(xoffset, yoffset);

	/*
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
	rotationMatrix *= Quaternion::multiply(qY, qX).toMatrixUnit();*/
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion