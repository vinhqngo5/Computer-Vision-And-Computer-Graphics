#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core.hpp>
#include <opencv2/core/opengl.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include "stb_image.h"
#include "shader.h"
//#include "camera.h"

#include <iostream>

using namespace cv;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;

// Uniform Buffer for Matrices
// this buffer will contain 3 matrices: projection, view and model
// each matrix is a float array with 16 components
GLuint matricesUniBuffer;
#define MatricesUniBufferSize sizeof(float) * 16 * 3
#define ProjMatrixOffset 0
#define ViewMatrixOffset sizeof(float) * 16
#define ModelMatrixOffset sizeof(float) * 16 * 2
#define MatrixSize sizeof(float) * 16

VideoCapture cap(0);

// ----------------------------------------------------
// Model Matrix 
//
// Copies the modelMatrix to the uniform buffer


void setModelMatrix() {

	glm::mat4 model = glm::mat4(1.0f);
	glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER,
		ModelMatrixOffset, MatrixSize, glm::value_ptr(model));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// ----------------------------------------------------
// Projection Matrix 
//
// Computes the projection Matrix and stores it in the uniform buffer
//float projMatrix[16];
static Mat_<float> projMatrix;
void buildProjectionMatrix(float nearp, float farp) {
	projMatrix.create(4, 4); projMatrix.setTo(0);

	float f_x = camera_matrix.at<double>(0, 0);
	float f_y = camera_matrix.at<double>(1, 1);

	float c_x = camera_matrix.at<double>(0, 2);
	float c_y = camera_matrix.at<double>(1, 2);

	projMatrix.at<float>(0, 0) = 2 * f_x / (float)SCR_WIDTH;
	projMatrix.at<float>(1, 1) = 2 * f_y / (float)SCR_HEIGHT;

	projMatrix.at<float>(2, 0) = 1.0f - 2 * c_x / (float)SCR_WIDTH;
	projMatrix.at<float>(2, 1) = 2 * c_y / (float)SCR_HEIGHT - 1.0f;
	projMatrix.at<float>(2, 2) = -(farp + nearp) / (farp - nearp);
	projMatrix.at<float>(2, 3) = -1.0f;

	projMatrix.at<float>(3, 2) = -2.0f*farp*nearp / (farp - nearp);

	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ProjMatrixOffset, MatrixSize, projMatrix.data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}


// ----------------------------------------------------
// View Matrix
//
// Computes the viewMatrix and stores it in the uniform buffer
//
// note: it assumes the camera is not tilted, 
// i.e. a vertical up vector along the Y axis (remember gluLookAt?)
//

void setCamera(cv::Mat viewMatrix) {
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, ViewMatrixOffset, MatrixSize, (float*)viewMatrix.data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int main()
{
    
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
														 // glfw window creation
														 // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// build and compile our shader zprogram
	// ------------------------------------
	Shader texShader("texture.vs", "texture.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float tex_vertices[] = {
		// positions          // colors           // texture coords
		1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int TVBO, TVAO, TEBO;
	glGenVertexArrays(1, &TVAO);
	glGenBuffers(1, &TVBO);
	glGenBuffers(1, &TEBO);

	glBindVertexArray(TVAO);

	glBindBuffer(GL_ARRAY_BUFFER, TVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_vertices), tex_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//enable depth test
	glEnable(GL_DEPTH_TEST);

	Shader ourShader("shader.vs", "shader.fs");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float cube_w = 1.0f;
	float vertices[] = {
		-cube_w, -cube_w, 0, 0.0f, 0.0f,
		cube_w, -cube_w, 0, 1.0f, 0.0f,
		cube_w, cube_w, 0, 1.0f, 1.0f,
		cube_w, cube_w, 0, 1.0f, 1.0f,
		-cube_w, cube_w, 0, 0.0f, 1.0f,
		-cube_w, -cube_w, 0, 0.0f, 0.0f,

		-cube_w, -cube_w, 2*cube_w, 0.0f, 0.0f,
		cube_w, -cube_w, 2*cube_w, 1.0f, 0.0f,
		cube_w, cube_w, 2*cube_w, 1.0f, 1.0f,
		cube_w, cube_w, 2*cube_w, 1.0f, 1.0f,
		-cube_w, cube_w, 2*cube_w, 0.0f, 1.0f,
		-cube_w, -cube_w, 2*cube_w, 0.0f, 0.0f,

		-cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,
		-cube_w, cube_w, 0, 1.0f, 1.0f,
		-cube_w, -cube_w, 0, 0.0f, 1.0f,
		-cube_w, -cube_w, 0, 0.0f, 1.0f,
		-cube_w, -cube_w, 2*cube_w, 0.0f, 0.0f,
		-cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,

		cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,
		cube_w, cube_w, 0, 1.0f, 1.0f,
		cube_w, -cube_w, 0, 0.0f, 1.0f,
		cube_w, -cube_w, 0, 0.0f, 1.0f,
		cube_w, -cube_w, 2*cube_w, 0.0f, 0.0f,
		cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,

		-cube_w, -cube_w, 0, 0.0f, 1.0f,
		cube_w, -cube_w, 0, 1.0f, 1.0f,
		cube_w, -cube_w, 2*cube_w, 1.0f, 0.0f,
		cube_w, -cube_w, 2*cube_w, 1.0f, 0.0f,
		-cube_w, -cube_w, 2*cube_w, 0.0f, 0.0f,
		-cube_w, -cube_w, 0, 0.0f, 1.0f,

		-cube_w, cube_w, 0, 0.0f, 1.0f,
		cube_w, cube_w, 0, 1.0f, 1.0f,
		cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,
		cube_w, cube_w, 2*cube_w, 1.0f, 0.0f,
		-cube_w, cube_w, 2*cube_w, 0.0f, 0.0f,
		-cube_w, cube_w, 0, 0.0f, 1.0f };
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//
	// Uniform Block
	//
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(ourShader.ID, "Matrices");
	glUniformBlockBinding(ourShader.ID, uniformBlockIndex, 0);

	glGenBuffers(1, &matricesUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUniBuffer);
	glBufferData(GL_UNIFORM_BUFFER, MatricesUniBufferSize, NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, matricesUniBuffer, 0, MatricesUniBufferSize); //setUniforms();
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// cube texture
	// load and create a texture 
	// -------------------------
	unsigned int texture1;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);

	buildProjectionMatrix(0.01f, 1000.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		
		// input
		// -----
		processInput(window);

		// load and create a texture 
		// -------------------------
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
											   // set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		Mat frame;
		cap >> frame;
		
		detectArucoMarkers(frame);
		//findAndComputePose(frame);
		cv::flip(frame, frame, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind Texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// render container
		texShader.use();

		glBindVertexArray(TVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glClear(GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		setCamera(viewMatrix);

		setModelMatrix();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// activate shader
		ourShader.use();

		// render box
		if (is_mark)
		{
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &TVAO);
	glDeleteBuffers(1, &TVBO);
	glDeleteBuffers(1, &TEBO);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}