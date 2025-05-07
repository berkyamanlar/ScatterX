#include "Mesh.h"
#include "rapidobj.hpp"

Mesh::Mesh(const std::string& Path) :
    position(0.0f),
    rotation(0.0f),
    scale(1.0f)
{
    LoadObjectModelFromDisk(Path);
    UpdateModelMatrix();
}

Mesh::Mesh() :
    position(0.0f),
    rotation(0.0f),
    scale(1.0f)
{
    UpdateModelMatrix();
}

Mesh::~Mesh()
{
    this->Clean();
    if(!this->fileName.empty()) std::cout << "Object " << this->fileName << " destroyed." << std::endl;
}

void Mesh::UpdateModelMatrix()
{
    // Start with identity matrix
    modelMatrix = glm::mat4(1.0f);

    // Apply transformations in order: scale -> rotate -> translate
    // 1. Translate
    modelMatrix = glm::translate(modelMatrix, position);

    // 2. Rotate (in degrees, convert to radians for GLM)
    // Use quaternions for better rotation interpolation and to avoid gimbal lock
    glm::quat rotationQuat = glm::quat(glm::radians(rotation));
    glm::mat4 rotationMatrix = glm::toMat4(rotationQuat);
    modelMatrix = modelMatrix * rotationMatrix;

    // 3. Scale
    modelMatrix = glm::scale(modelMatrix, scale);

    CalculateDimensions();
}

void Mesh::Clean()
{
    // Free the GPU resources (VAO, VBO, EBO)
    if (VAO_obj != 0) {
        glDeleteVertexArrays(1, &VAO_obj);
        VAO_obj = 0;  // Set to 0 to prevent double-deletion
    }
    if (VBO_obj != 0) {
        glDeleteBuffers(1, &VBO_obj);
        VBO_obj = 0;  // Set to 0 to prevent double-deletion
    }
    if (EBO_obj != 0) {
        glDeleteBuffers(1, &EBO_obj);
        EBO_obj = 0;  // Set to 0 to prevent double-deletion
    }

    // Clean up the shader program
    if (objectShaderProgram) {
        // Resetting the unique_ptr will automatically release the shader resource
        objectShaderProgram.reset();
    }

    // Free dynamic memory in containers
    vertices.clear();
    vertices.shrink_to_fit();  // Release memory allocated by the vector

    indices.clear();
    indices.shrink_to_fit();  // Release memory allocated by the vector
}

void Mesh::LoadObjectModelFromDisk(const std::string& Path)
{
    rapidobj::Result result = rapidobj::ParseFile(Path);
    if (result.error) {
        std::cout << "Error loading OBJ: " << result.error.code.message() << '\n';
        return;
    }
    bool success = rapidobj::Triangulate(result);
    if (!success) {
        std::cout << "Error triangulating OBJ: " << result.error.code.message() << '\n';
        return;
    }
    this->fileName = this->extractFilename(Path);

    // Clear previous data just in case
    vertices.clear();
    indices.clear();

    const auto& attrib = result.attributes;
    const auto& shapes = result.shapes;

    // Pre-reserve space to avoid reallocations
    size_t estimatedVertexCount = attrib.positions.size() / 3;
    vertices.reserve(estimatedVertexCount);

    // More efficient than string-based approach - using a custom hash function
    struct VertexData {
        float x, y, z;  // position

        bool operator==(const VertexData& other) const {
            // Use a small epsilon for floating point comparison
            constexpr float EPSILON = 1e-7f;
            return std::abs(x - other.x) < EPSILON &&
                std::abs(y - other.y) < EPSILON &&
                std::abs(z - other.z) < EPSILON;
        }
    };

    struct VertexDataHash {
        size_t operator()(const VertexData& v) const {
            // Simple but effective hash function for floating point values
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<VertexData, unsigned int, VertexDataHash> uniqueVertices;
    uniqueVertices.reserve(estimatedVertexCount);

    // Pre-allocate indices space
    size_t totalIndexCount = 0;
    for (const auto& shape : shapes) {
        totalIndexCount += shape.mesh.indices.size();
    }
    indices.reserve(totalIndexCount);

    for (const auto& shape : shapes)
    {
        for (const auto& idx : shape.mesh.indices)
        {
            // Get vertex data
            VertexData vdata = {
                attrib.positions[3 * idx.position_index + 0],
                attrib.positions[3 * idx.position_index + 1],
                attrib.positions[3 * idx.position_index + 2]
            };

            // Create vertex with colors
            glm::vec3 position(vdata.x, vdata.y, vdata.z);
            glm::vec3 color(1.0f, 1.0f, 1.0f);

            // Try to find existing vertex
            auto it = uniqueVertices.find(vdata);
            if (it == uniqueVertices.end()) {
                // Not found, add new vertex
                unsigned int newIndex = static_cast<unsigned int>(vertices.size());
                uniqueVertices.emplace(vdata, newIndex);
                vertices.push_back({ position, color });
                indices.push_back(newIndex);

            }
            else {
                // Use existing vertex
                indices.push_back(it->second);
            }
        }
    }
    this->numTriangles = indices.size() / 3;
    this->modelMemoryMB = (vertices.size() * sizeof(Vertex) + indices.size() * sizeof(GLuint)) / (1024.0 * 1024.0);
    this->UpdateTriangleData();
    this->CalculateDimensions();
    std::cout << "Object " << this->fileName << " created." << std::endl;
}

void Mesh::CalculateDimensions()
{
    // Check if we have any vertices
    if (vertices.empty()) {
        height = 0.0f;
        length = 0.0f;
        return;
    }

    // Calculate the bounding box in model space
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    for (const auto& vertex : vertices) {
        minBounds.x = std::min(minBounds.x, vertex.position.x);
        minBounds.y = std::min(minBounds.y, vertex.position.y);
        minBounds.z = std::min(minBounds.z, vertex.position.z);

        maxBounds.x = std::max(maxBounds.x, vertex.position.x);
        maxBounds.y = std::max(maxBounds.y, vertex.position.y);
        maxBounds.z = std::max(maxBounds.z, vertex.position.z);
    }

    // Calculate dimensions
    glm::vec3 dimensions = maxBounds - minBounds;

    // Height is along the Y axis
    height = dimensions.y * scale.y;

    // Length is the maximum dimension in the XZ plane
    length = std::max(dimensions.x * scale.x, dimensions.z * scale.z);
}

std::string Mesh::extractFilename(const std::string& path) {
    // Find the last directory separator
    size_t lastSlash = path.find_last_of("/\\");

    // If no separator found, return the whole string
    if (lastSlash == std::string::npos) {
        return path;
    }

    // Return everything after the last separator
    return path.substr(lastSlash + 1);
}

void Mesh::UpdateTriangleData()
{
    triangles.clear();
    triangles.reserve(indices.size() / 3);

    // Create a triangle object for each triangle in the mesh
    for (size_t i = 0; i < indices.size(); i += 3) {
        Triangle tri(indices[i], indices[i + 1], indices[i + 2]);

        // Calculate triangle center
        const glm::vec3& v1 = vertices[indices[i]].position;
        const glm::vec3& v2 = vertices[indices[i + 1]].position;
        const glm::vec3& v3 = vertices[indices[i + 2]].position;

        // Calculate triangle normal
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        tri.normal = glm::normalize(glm::cross(edge1, edge2));

        triangles.push_back(tri);
    }
}

void Mesh::SetTriangleSelected(size_t triangleIndex, bool selection)
{
    if (triangleIndex >= triangles.size()) return;
    this->triangles[triangleIndex].selected = selection;
}
