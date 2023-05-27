#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>

using namespace std;

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct Vertex {
	GLfloat x, y, z, r = 1.0, g = 0.0, b = 1.0;
};

struct Face {
	GLint v1, v2, v3;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void readFromFile(string filename, vector<Vertex>& vertices, vector<Face>& faces);

void buildVertices(vector<GLfloat>& finalVertices, vector<Vertex> vertices, vector<Face> faces);

int setupShader();
int setupObj();

const GLuint WIDTH = 1500, HEIGHT = 1500;

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"

"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
vector<Vertex> vertices;
vector<GLfloat> finalVertices;
vector<Face> faces;

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Atividade Vivencial 1", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	GLuint VAO = setupObj();

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1); 
		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			
		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		}

		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));


		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, finalVertices.size());
		
		glDrawArrays(GL_LINE, 0, finalVertices.size());
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

}

void readFromFile(string filename, vector<Vertex>& vertices, vector<Face>& faces)
{
	string line;

	ifstream file(filename);
	
	while (getline(file, line))
	{
		if (line.substr(0, 2) == "v ")
		{
			istringstream s(line.substr(2));
			
			Vertex vertex;
			s >> vertex.x >> vertex.y >> vertex.z;
			
			vertices.push_back(vertex);
		}
		else if (line.substr(0, 2) == "f ")
		{
			istringstream s(line.substr(2));

			Face face;
			string v1, v2, v3;
			
			s >> v1 >> v2 >> v3;

			face.v1 = stoi(v1);
			face.v2 = stoi(v2);
			face.v3 = stoi(v3);

			faces.push_back(face);
		}
	}

	file.close();
}

void buildVertices(vector<GLfloat>& finalVertices, vector<Vertex> vertices, vector<Face> faces) {
	for (size_t i = 0; i < faces.size(); i++) {

		int v1Position = faces[i].v1 - 1;
		finalVertices.push_back(vertices[v1Position].x);
		finalVertices.push_back(vertices[v1Position].y);
		finalVertices.push_back(vertices[v1Position].z);
		finalVertices.push_back(vertices[v1Position].r);
		finalVertices.push_back(vertices[v1Position].g);
		finalVertices.push_back(vertices[v1Position].b);

		int v2Position = faces[i].v2 - 1;
		finalVertices.push_back(vertices[v2Position].x);
		finalVertices.push_back(vertices[v2Position].y);
		finalVertices.push_back(vertices[v2Position].z);
		finalVertices.push_back(vertices[v2Position].r);
		finalVertices.push_back(vertices[v2Position].g);
		finalVertices.push_back(vertices[v2Position].b);

		int v3Position = faces[i].v3 - 1;
		finalVertices.push_back(vertices[v3Position].x);
		finalVertices.push_back(vertices[v3Position].y);
		finalVertices.push_back(vertices[v3Position].z);
		finalVertices.push_back(vertices[v3Position].r);
		finalVertices.push_back(vertices[v3Position].g);
		finalVertices.push_back(vertices[v3Position].b);
	}
}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupObj()
{
	readFromFile("suzanneTriLowPoly.obj", vertices, faces);
	
	buildVertices(finalVertices, vertices, faces);

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, finalVertices.size() * sizeof(GLfloat), finalVertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	return VAO;
}
