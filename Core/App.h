#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

#include <nvml.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "Renderer.h"

enum class MeshType {
    Plane, Cube, Sphere, Cylinder, Disk, Trihedral, Dihedral, Picker, AI
};

class Application {
public:
    Application();
    ~Application();

    void Run();

private:
    void Init();
    void MainLoop();
    void Shutdown();

    GLuint LoadTextureFromFile(const char* filename, int* out_width, int* out_height);
    void SetGeometryToOrigin(int meshIndex);
    
    // Create mesh objects
    void loadMesh(MeshType type);
    void renderMeshCreator();
    void createPlaneMesh(std::string& meshName);
    void createCubeMesh(std::string& meshName, const glm::vec3& center, const glm::vec3& dimensions, int LOD);
    void createSphereMesh(std::string& meshName, glm::vec3& center, float radius, int LOD);
    void createCylinderMesh(std::string& meshName, glm::vec3& center, float radius, float height, int LOD);
    void createDiskMesh(std::string& meshName, glm::vec3& center, float radius, int LOD, int axis);
    void createTrihedralReflectorMesh(std::string& meshName, glm::vec3& center, float size);
    void createDihedralReflectorMesh(std::string& meshName, glm::vec3& center, float size);

    // Performance metrics
    float GetCPUutilization();
    std::string GetGPUutilization();
    std::string GetRAMutilization();

    // Panels
    void drawContentBrowser();
    void drawResultsPanel();
    void drawSceneCollection();
    void drawSceneInspector();
    void drawObjectEditor();
    void renderObjectSelectionWindow();
    void drawTitleBar();
    void drawCoordinateSystemImage();
    void drawPerformanceMetrics(float fps, float cpuUsage, std::string ramUsage, std::string gpuUsage);

    std::unique_ptr<Renderer> renderer;

    // GLFW window
    GLFWwindow* window;

    // Loaded icons
    GLuint folderIconTextureID;
    GLuint windowIconTextureID;
    GLuint undoButtonTextureID;
    GLuint resetIconTextureID;
    GLuint objIconTextureID;
    GLuint coordinateSystemTextureID;

    // Title bar buttons boolean variables
    bool m_showMeshOptions = false;
    bool m_showSceneOptions = false;

    // Inside scene options
    bool m_showGrid = true;
    bool m_showCoordinateSystem = true;
    bool m_showAxisOrientation = true;
    bool m_showSceneInspector = true;
    bool m_showPerformanceMetrics = true;

    // Inside mesh options
    int m_LOD = 10;         // Level of detail (grid size)
    // Plane properties
    bool m_showPlaneCreator = false;
    int m_planeAxis = 0; // 0 = X, 1 = Y, 2 = Z
    float m_planeStart = -5.0f;
    float m_planeEnd = 5.0f;
    float m_planeStart2 = -5.0f; // Second dimension
    float m_planeEnd2 = 5.0f;    // Second dimension
    // Cube properties
    bool m_showCubeCreator = false; 
    glm::vec3 m_cubeCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_cubeDimensions = glm::vec3(1.0f, 1.0f, 1.0f); // Dimensions of the cube (width, height, depth)
    // Sphere properties
    bool m_showSphereCreator = false;
    glm::vec3 m_sphereCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_sphereRadius = 1.0f;
    // Cylinder properties
    bool m_showCylinderCreator = false;
    glm::vec3 m_cylinderCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_cylinderRadius = 1.0f;
    float m_cylinderHeight = 1.0f;
    // Disk properties
    bool m_showDiskCreator = false;
    glm::vec3 m_diskCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_diskRadius = 1.0f;
    int m_diskAxis = 0; // 0 = X, 1 = Y, 2 = Z
    // Trihedral Corner Reflector properties
    bool m_showTrihedralCreator = false;
    glm::vec3 m_trihedralCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_trihedralSize = 1.0f;
    // Dihedral Corner Reflector properties
    bool m_showDihedralCreator = false;
    glm::vec3 m_dihedralCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_dihedralSize = 1.0f;

    // Content browser 
    std::string contentBrowserPath = "./Database";
    bool isInRootDirectory = true;

    float deltaTime;
    float lastFrame;

    std::string selectedObjectNameSceneCollection = "";
    std::string selectedItemPathContentBrowser = "";
};
