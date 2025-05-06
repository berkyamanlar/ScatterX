#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Camera.h"

// Structure to standardize the vertices used in the meshes
struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	bool selected = false;
};

class Mesh
{
public:
	// Initializes the mesh
	Mesh(const std::string& Path);
	Mesh();
	~Mesh();

	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;

	void Clean();
	void LoadObjectModelFromDisk(const std::string& Path);
	void CalculateDimensions();
	std::string extractFilename(const std::string& path);

	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	
	std::string fileName;
	size_t numTriangles = 0;
	float modelMemoryMB;
	float length = 0;
	float height = 0;
	bool isVisible = true;

	// Transformation methods
	void UpdateModelMatrix();
	glm::mat4 GetModelMatrix() const { return modelMatrix; }

	// Transform properties
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f); // In degrees
	glm::vec3 scale = glm::vec3(1.0f);

	// Object
	std::unique_ptr<Shader> objectShaderProgram;
	GLuint VAO_obj, VBO_obj, EBO_obj;

private:
	glm::mat4 modelMatrix = glm::mat4(1.0f);
};
#endif