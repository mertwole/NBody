#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "LoadFile.h"
#include "ShaderProgram.h"

#include "include/cl.h"
#include "include/cl_gl.h"
#include "include/glad.h"
#include "include/glfw3.h"

#include <windows.h>

using namespace std;

const int bodies_count = 5000;
const float delta_time = 0.00001f;

const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 1000;

// CL objects.
cl_kernel kernel;
cl_program program;
cl_device_id device;
cl_mem bodies_buf_cl;
cl_command_queue commandQueue;
cl_context context;
size_t global_work_size[1] = { bodies_count };

// GL objects
GLuint bodies_buf;

struct body {
	GLfloat pos_x;
	GLfloat pos_y;
	GLfloat vel_x;
	GLfloat vel_y;

	GLfloat mass;
	GLfloat pad0;
	GLfloat pad1;
	GLfloat pad2;
};

void get_bodies(body* bodies) {
	for (int i = 0; i < bodies_count; i++) {
		bodies[i].mass = 0.01f;
		bodies[i].pos_x = -0.7f + 1.4f * (float)i / (float)bodies_count;
		bodies[i].pos_y = 0.0f;
		bodies[i].vel_y = 100 * (0.2 + 0.2 * (float)i / (float)bodies_count);
		bodies[i].vel_x = 0.0f;
	}

	bodies[bodies_count / 2].mass = 1000;
	bodies[bodies_count / 2].vel_y = 0.0f;
}

//**********************CL functions*******************************

cl_platform_id cl_choose_platform() {
	cl_uint numPlatforms;
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS) {
		cout << "error getting platforms" << endl;
		exit(1);
	}
	if (numPlatforms == 0) {
		cout << "no OpenCL platforms found" << endl;
		exit(1);
	}

	cl_platform_id platform = NULL;
	clGetPlatformIDs(1, &platform, NULL);
	return platform;
}

void cl_create_program(const char* filename, cl_context context) {
	string sourceStr;
	load_file(filename, &sourceStr);
	const char* source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
}

void cl_build_program() {
	clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	cl_build_status build_status;
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
	if (build_status == CL_BUILD_ERROR) {
		char log[1024];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 1024 * sizeof(char), &log, NULL);
		cout << "build failed, log : " << log << endl;
		exit(1);
	}
}

void set_kernel_args() {
	// 0th
	cl_int* err = NULL;
	bodies_buf_cl = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, bodies_buf, err);
	if (err) {
		cout << "error during initializing buffer" << endl;
		exit(1);
	}

	clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bodies_buf_cl);
	// 1th
	cl_float dt = delta_time;
	clSetKernelArg(kernel, 1, sizeof(cl_float), &dt);
}

void cl_init()
{
	cl_platform_id platfrom = cl_choose_platform();
	// Load pointer to clGetGLContextInfoKHR
	cl_int(*_clGetGLContextInfoKHR)(
		const cl_context_properties * properties,
		cl_gl_context_info  param_name,
		size_t  param_value_size,
		void* param_value,
		size_t * param_value_size_ret
	);
	_clGetGLContextInfoKHR = 
	(cl_int(*)(const cl_context_properties*, cl_gl_context_info, size_t, void*, size_t*))
	clGetExtensionFunctionAddressForPlatform(platfrom, "clGetGLContextInfoKHR");
	// Windows only
	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platfrom,
		0 
	};

	_clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), &device, NULL);

	context = clCreateContext(properties, 1, &device, NULL, NULL, NULL);
	commandQueue = clCreateCommandQueue(context, device, 0, NULL);
	cl_create_program("kernel.cl", context);
	cl_build_program();
	kernel = clCreateKernel(program, "move", NULL);
	set_kernel_args();
}

void cl_compute() {
	clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
	clFinish(commandQueue);
}

void cl_cleanup() {
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseMemObject(bodies_buf_cl);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
}

//**********************GL functions*******************************

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow* gl_create_window() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		throw -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		throw -1;
	}

	return window;
}

void gl_init() {
	GLuint render_shader = ShaderProgram()
		.AddFrag("shaders/draw_bodies.frag")
		.AddVert("shaders/draw_bodies.vert")
		.Compile();
	glUseProgram(render_shader);

	body bodies[bodies_count];
	get_bodies(bodies);
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &bodies_buf);
	glBindBuffer(GL_ARRAY_BUFFER, bodies_buf);
	glBufferData(GL_ARRAY_BUFFER, bodies_count * sizeof(body), (void*)bodies, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(body), (void*)0);
	glEnableVertexAttribArray(0);
}

void gl_draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_POINTS, 0, bodies_count);
}

//******************************************************************

int main()
{
	GLFWwindow* window = gl_create_window();

	// Before cl_init() because creates rendering context
	gl_init();

	cl_init();

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		cl_compute();
		gl_draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	cl_cleanup();

	return 0;
}