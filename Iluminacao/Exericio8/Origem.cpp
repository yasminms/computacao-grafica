#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "stb_image.h"

#include "Shader.h"

struct Vertex {
	GLfloat x, y, z, r = 1.0, g = 0.0, b = 1.0;
};

struct Face {
	string v1, v2, v3, t1, t2, t3, n1, n2, n3;
};

struct Texture {
	GLfloat t1, t2;
};

struct Normal {
	GLfloat n1, n2, n3;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

string getTextureFile(string filename);

void readFromObjFile(string filename, vector<Vertex>& vertices, vector<Face>& faces, vector<Texture>& textures, vector<Normal>& normals);

void buildVertices(vector<GLfloat>& finalVertices);

int setupGeometry();

const GLuint WIDTH = 1500, HEIGHT = 1500;

bool rotateX=false, rotateY=false, rotateZ=false;
vector<Vertex> vertices;
vector<GLfloat> finalVertices;
vector<GLfloat> finalTextures;
vector<Face> faces;
vector<Texture> textures;
vector<Normal> normals;

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Iluminação", nullptr, nullptr);
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

	Shader shader("../shaders/sprite.vs", "../shaders/sprite.fs");

	GLuint VAO = setupGeometry();

	glUseProgram(shader.ID);

	glUniform1i(glGetUniformLocation(shader.ID, "tex_buffer"), 0);

	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("view", value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	shader.setMat4("projection", glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");

	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	shader.setFloat("ka", 0.2);
	shader.setFloat("kd", 0.5);
	shader.setFloat("ks", 0.5);
	shader.setFloat("q", 10.0);

	shader.setVec3("lightPos", -2.0, 10.0, 2.0);
	shader.setVec3("lightColor", 1.0, 1.0, 0.0);

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

string getTextureFile(string filename)
{
	string line, path;

	ifstream file(filename);

	while (getline(file, line))
	{
		if (line.empty()) {
			continue;
		}

		if (line.substr(0, 6).compare("map_Kd") == 0)
		{
			path = line.substr(7);
		}
	}

	if (path.empty()) {
		cout << "Arquivo .mtl não contém caminho para a textura." << endl;
	}

	file.close();

	return path;
}

void readFromObjFile(string filename, vector<Vertex>& vertices, vector<Face>& faces, vector<Texture>& textures, vector<Normal>& normals)
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
			replace(line.begin(), line.end(), '/', ' ');

			istringstream s(line.substr(2));

			Face face;
			string v1, v2, v3, t1, t2, t3, n1, n2, n3;
			
			s >> v1 >> t1 >> n1 >> v2 >> t2 >> n2 >> v3 >> t3 >> n3;

			face.v1 = v1;
			face.v2 = v2;
			face.v3 = v3;

			face.t1 = t1;
			face.t2 = t2;
			face.t3 = t3;

			face.n1 = n1;
			face.n2 = n2;
			face.n3 = n3;

			faces.push_back(face);
		}
		else if (line.substr(0, 3) == "vt ")
		{
			istringstream s(line.substr(2));

			Texture texture;
			GLfloat t1, t2;

			s >> t1 >> t2;

			texture.t1 = t1;
			texture.t2 = t2;

			textures.push_back(texture);
		}
		else if (line.substr(0, 3) == "vn ")
		{
			istringstream s(line.substr(2));

			Normal normal;
			GLfloat n1, n2, n3;

			s >> n1 >> n2 >> n3;

			normal.n1 = n1;
			normal.n2 = n2;
			normal.n3 = n3;

			normals.push_back(normal);
		}
	}

	file.close();
}

void setVertexPosition(vector<GLfloat>& finalVertices, int vertexPosition, int texturePosition, int normalPosition) {
	finalVertices.push_back(vertices[vertexPosition].x);
	finalVertices.push_back(vertices[vertexPosition].y);
	finalVertices.push_back(vertices[vertexPosition].z);
	finalVertices.push_back(vertices[vertexPosition].r);
	finalVertices.push_back(vertices[vertexPosition].g);
	finalVertices.push_back(vertices[vertexPosition].b);
	finalVertices.push_back(textures[texturePosition].t1);
	finalVertices.push_back(textures[texturePosition].t2);
	finalVertices.push_back(normals[normalPosition].n1);
	finalVertices.push_back(normals[normalPosition].n2);
	finalVertices.push_back(normals[normalPosition].n3);
}

void buildVertices(vector<GLfloat>& finalVertices) {
	for (size_t i = 0; i < faces.size(); i++) {
		int v1Position = stoi(faces[i].v1) - 1;
		int t1Position = stoi(faces[i].t1) - 1;
		int n1Position = stoi(faces[i].n1) - 1;
		setVertexPosition(finalVertices, v1Position, t1Position, n1Position);

		int v2Position = stoi(faces[i].v2) - 1;
		int t2Position = stoi(faces[i].t2) - 1;
		int n2Position = stoi(faces[i].n2) - 1;
		setVertexPosition(finalVertices, v2Position, t2Position, n2Position);


		int v3Position = stoi(faces[i].v3) - 1;
		int t3Position = stoi(faces[i].t3) - 1;
		int n3Position = stoi(faces[i].n3) - 1;
		setVertexPosition(finalVertices, v3Position, t3Position, n3Position);
	}
}

int setupGeometry()
{
	readFromObjFile("../textures/SuzanneTriTextured.obj", vertices, faces, textures, normals);
	
	buildVertices(finalVertices);

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, finalVertices.size() * sizeof(GLfloat), finalVertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	return VAO;
}
