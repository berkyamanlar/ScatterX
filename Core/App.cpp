#include "App.h"
#include "stb/stb_image.h"

Application::Application()
    : window(nullptr), deltaTime(0.0f), lastFrame(0.0f)
{
    Init();
}

Application::~Application() {
    Shutdown();
}

GLuint Application::LoadTextureFromFile(const char* filename, int* out_width, int* out_height) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return 0;
    }
    // Create a new OpenGL texture ID
    GLuint textureID;
    glGenTextures(1, &textureID);
    // Bind the texture so we can operate on it
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Upload the image data to the GPU as a 2D texture
    // - target: GL_TEXTURE_2D (standard 2D texture)
    // - level: 0 (base level, no mipmap)
    // - internalFormat: GL_RGBA (store texture internally as RGBA)
    // - width, height: image dimensions
    // - border: 0 (must always be 0)
    // - format: GL_RGBA (format of incoming image data)
    // - type: GL_UNSIGNED_BYTE (data is 8-bit per channel)
    // - data: pointer to image pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Set texture filtering options
    // GL_LINEAR: smooth interpolation for scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear filtering for downsizing
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear filtering for upsizing

    stbi_image_free(data);

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;

    return textureID;
}

void Application::SetGeometryToOrigin(int meshIndex)
{
    if (meshIndex < 0 || meshIndex >= renderer->sceneCollectionMeshes.size()) {
        return; // Invalid mesh index
    }

    Mesh& mesh = renderer->sceneCollectionMeshes[meshIndex];

    // Calculate the bounding box of the mesh to find its center
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    // Find the minimum and maximum coordinates of all vertices
    for (const auto& vertex : mesh.vertices) {
        // Apply current scale to vertex positions when calculating bounds
        glm::vec3 scaledPos = vertex.position * mesh.scale;

        minBounds.x = std::min(minBounds.x, scaledPos.x);
        minBounds.y = std::min(minBounds.y, scaledPos.y);
        minBounds.z = std::min(minBounds.z, scaledPos.z);

        maxBounds.x = std::max(maxBounds.x, scaledPos.x);
        maxBounds.y = std::max(maxBounds.y, scaledPos.y);
        maxBounds.z = std::max(maxBounds.z, scaledPos.z);
    }

    // Calculate the center of the object in XZ plane
    glm::vec3 objectCenter;
    objectCenter.x = (minBounds.x + maxBounds.x) * 0.5f;
    objectCenter.z = (minBounds.z + maxBounds.z) * 0.5f;

    // For Y, use the minimum bound so the bottom of the object sits at y=0
    objectCenter.y = minBounds.y;

    // Calculate the current world-space center by considering current rotation
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mesh.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mesh.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(mesh.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 rotatedCenter = glm::vec3(rotationMatrix * glm::vec4(objectCenter, 1.0f));

    // Set the position to the negative of the rotated center to move the object
    // so its bottom is at y=0 and centered in xz plane
    mesh.position = -rotatedCenter;

    // Update the model matrix to apply the transformation
    mesh.UpdateModelMatrix();
}

void Application::loadMesh(MeshType type) {
    switch (type) {
    case MeshType::Plane:
        m_showPlaneCreator = true;
        m_showCubeCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = false;
        m_showDiskCreator = false;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Cube:
        m_showCubeCreator = true;
        m_showPlaneCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = false;
        m_showDiskCreator = false;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Sphere:
        m_showCubeCreator = false;
        m_showPlaneCreator = false;
        m_showSphereCreator = true;
        m_showCylinderCreator = false;
        m_showDiskCreator = false;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Cylinder:
        m_showCubeCreator = false;
        m_showPlaneCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = true;
        m_showDiskCreator = false;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Disk:
        m_showCubeCreator = false;
        m_showPlaneCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = false;
        m_showDiskCreator = true;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Trihedral:
        m_showCubeCreator = false;
        m_showPlaneCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = false;
        m_showDiskCreator = false;
        m_showTrihedralCreator = true;
        m_showDihedralCreator = false;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    case MeshType::Dihedral:
        m_showCubeCreator = false;
        m_showPlaneCreator = false;
        m_showSphereCreator = false;
        m_showCylinderCreator = false;
        m_showDiskCreator = false;
        m_showTrihedralCreator = false;
        m_showDihedralCreator = true;
        m_showMeshOptions = false; // Close the mesh selection menu
        break;
    default:
        break;
    }
}

void Application::renderMeshCreator() {
    if (m_showPlaneCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (ImGui::Begin("##HiddenTitle", &m_showPlaneCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Plane Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            const char* axisLabels[] = { "X-Axis (YZ Plane)", "Y-Axis (XZ Plane)", "Z-Axis (XY Plane)" };
            ImGui::Text("Choose plane orientation:");

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::Combo("Orientation", &m_planeAxis, axisLabels, IM_ARRAYSIZE(axisLabels));
            ImGui::PopStyleColor(4);

            ImGui::Spacing(); ImGui::Spacing();

            const char* dim1Label = "Dimension 1";
            const char* dim2Label = "Dimension 2";

            switch (m_planeAxis) {
            case 0: dim1Label = "Y Range"; dim2Label = "Z Range"; break;
            case 1: dim1Label = "X Range"; dim2Label = "Z Range"; break;
            case 2: dim1Label = "X Range"; dim2Label = "Y Range"; break;
            }

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

            ImGui::Text("%s:", dim1Label);
            ImGui::SetNextItemWidth(160);
            ImGui::DragFloat("Start##1", &m_planeStart, 0.1f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(160);
            ImGui::DragFloat("End##1", &m_planeEnd, 0.1f);

            ImGui::Spacing();

            ImGui::Text("%s:", dim2Label);
            ImGui::SetNextItemWidth(160);
            ImGui::DragFloat("Start##2", &m_planeStart2, 0.1f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(160);
            ImGui::DragFloat("End##2", &m_planeEnd2, 0.1f);

            ImGui::Spacing(); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::Text("Level of Detail:");
            ImGui::SliderInt("LOD", &m_LOD, 1, 100);
            ImGui::Text("(Higher values create more detailed meshes)");
            ImGui::PopStyleColor(2);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "NewPlane";
            ImGui::InputText("##HiddenNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                // Ensure mesh name uniqueness
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                // Keep adding "_cp" until a unique name is found
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }


                createPlaneMesh(meshName);
                m_LOD = 10;
                m_showPlaneCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showPlaneCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleColor(3); // Inputs
            ImGui::End();
        }

        ImGui::PopStyleColor(4); // Window + text
    }
    if (m_showCubeCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        // Base theme colors
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Widget background colors (fix blue)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenCubeTitle", &m_showCubeCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Cube Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_cubeCenter), 0.1f);

            ImGui::Spacing();

            ImGui::Text("Size:");
            ImGui::DragFloat3("Dimensions", glm::value_ptr(m_cubeDimensions), 0.1f);

            ImGui::Spacing(); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::Text("Level of Detail:");
            ImGui::SliderInt("LOD", &m_LOD, 1, 100);
            ImGui::Text("(Higher values create more detailed meshes)");
            ImGui::PopStyleColor(2);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "NewCube";
            ImGui::InputText("##CubeNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            // Create button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                // Ensure mesh name uniqueness
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                // Keep adding "_cp" until a unique name is found
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }


                createCubeMesh(meshName, m_cubeCenter, m_cubeDimensions, m_LOD);
                m_LOD = 10;
                m_showCubeCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            // Cancel button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showCubeCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7); // 4 (window) + 3 (frame bg)
    }
    if (m_showSphereCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Widget background colors (fix blue)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenSphereTitle", &m_showSphereCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Sphere Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            // Position input
            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_sphereCenter), 0.1f);

            ImGui::Spacing();

            // Radius input
            ImGui::Text("Radius:");
            ImGui::DragFloat("Radius", &m_sphereRadius, 0.1f, 0.1f, 100.0f);

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            // Level of Detail (LOD) input
            ImGui::Text("Level of Detail:");
            ImGui::SliderInt("LOD", &m_LOD, 1, 100);
            ImGui::Text("(Higher values create more detailed meshes)");
            ImGui::PopStyleColor(2);

            ImGui::Spacing();
            ImGui::Spacing();

            static char nameBuffer[128] = "NewSphere";
            ImGui::InputText("##SphereNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            // Buttons for creating or cancelling the sphere creation
            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            // Create button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                // Ensure mesh name uniqueness
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                // Keep adding "_cp" until a unique name is found
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }


                // Call the function to create the sphere mesh
                createSphereMesh(meshName, m_sphereCenter, m_sphereRadius, m_LOD);
                m_LOD = 10;
                m_showSphereCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            // Cancel button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showSphereCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7);
    }
    if (m_showCylinderCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Widget background colors (fix blue)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenCylinderTitle", &m_showCylinderCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Cylinder Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_cylinderCenter), 0.1f);

            ImGui::Spacing();

            ImGui::Text("Radius:");
            ImGui::DragFloat("Radius", &m_cylinderRadius, 0.1f, 0.0f, 100.0f);

            ImGui::Spacing();

            ImGui::Text("Height:");
            ImGui::DragFloat("Height", &m_cylinderHeight, 0.1f, 0.0f, 100.0f);

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::Text("Level of Detail:");
            ImGui::SliderInt("LOD", &m_LOD, 3, 100); // LOD for cylinder
            ImGui::Text("(Higher values create more detailed meshes)");
            ImGui::PopStyleColor(2);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "NewCylinder";
            ImGui::InputText("##CylinderNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                // Ensure mesh name uniqueness
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                // Keep adding "_cp" until a unique name is found
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }


                // Create the cylinder mesh
                createCylinderMesh(meshName, m_cylinderCenter, m_cylinderRadius, m_cylinderHeight, m_LOD);
                m_LOD = 10;
                m_showCylinderCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showCylinderCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7);
    }
    if (m_showDiskCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Widget background colors
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenDiskTitle", &m_showDiskCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Disk Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_diskCenter), 0.1f);

            ImGui::Spacing();

            ImGui::Text("Radius:");
            ImGui::DragFloat("Radius", &m_diskRadius, 0.1f, 0.1f, 100.0f);

            ImGui::Spacing(); ImGui::Spacing();

            // Orientation Selection
            const char* axisLabels[] = { "X-Axis (YZ Plane)", "Y-Axis (XZ Plane)", "Z-Axis (XY Plane)" };
            ImGui::Text("Choose disk orientation:");
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::Combo("Orientation", &m_diskAxis, axisLabels, IM_ARRAYSIZE(axisLabels));
            ImGui::PopStyleColor(); // PopupBg

            ImGui::Spacing(); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            ImGui::Text("Level of Detail:");
            ImGui::SliderInt("LOD", &m_LOD, 1, 100);
            ImGui::Text("(Higher values create more detailed meshes)");
            ImGui::PopStyleColor(2);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "NewDisk";
            ImGui::InputText("##DiskNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }

                createDiskMesh(meshName, m_diskCenter, m_diskRadius, m_LOD, m_diskAxis);
                m_LOD = 10;
                m_showDiskCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showDiskCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7);
    }
    if (m_showTrihedralCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenTrihedralTitle", &m_showTrihedralCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Trihedral Corner Reflector");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_trihedralCenter), 0.1f);

            ImGui::Spacing();

            ImGui::Text("Size (length of each leg):");
            ImGui::DragFloat("Size", &m_trihedralSize, 0.1f, 0.1f, 100.0f);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "newTrihedral";
            ImGui::InputText("##TrihedralNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }

                createTrihedralReflectorMesh(meshName, m_trihedralCenter, m_trihedralSize);
                m_showTrihedralCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showTrihedralCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7);
    }
    if (m_showDihedralCreator) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 0));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Widget background colors (fix blue)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Begin("##HiddenDihedralTitle", &m_showDihedralCreator,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {

            ImGui::Text("Dihedral Corner Reflector Configuration");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Position:");
            ImGui::DragFloat3("Center", glm::value_ptr(m_dihedralCenter), 0.1f);

            ImGui::Spacing();

            ImGui::Text("Size:");
            ImGui::DragFloat("Size", &m_dihedralSize, 0.1f, 0.1f, 100.0f);

            ImGui::Spacing(); ImGui::Spacing();

            static char nameBuffer[128] = "NewDihedral";
            ImGui::InputText("##DihedralNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank);

            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            float windowWidth = ImGui::GetWindowWidth();
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0))) {
                // Ensure mesh name uniqueness
                std::string baseName = nameBuffer;
                std::string meshName = baseName + ".mesh";

                // Keep adding "_cp" until a unique name is found
                while (std::any_of(renderer->sceneCollectionMeshes.begin(), renderer->sceneCollectionMeshes.end(),
                    [&](const Mesh& mesh) { return mesh.fileName == meshName; })) {
                    baseName += "_cp";
                    meshName = baseName + ".mesh";
                }

                createDihedralReflectorMesh(meshName, m_dihedralCenter, m_dihedralSize);
                m_dihedralSize = 10;  // Reset size after creation
                m_showDihedralCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                m_showDihedralCreator = false;
            }

            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        ImGui::PopStyleColor(7);
    }

}

void Application::createPlaneMesh(std::string& meshName) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    // Calculate vertices based on the selected axis
    int segments = m_LOD;
    float dx1 = (m_planeEnd - m_planeStart) / segments;
    float dx2 = (m_planeEnd2 - m_planeStart2) / segments;

    // Generate vertices
    for (int i = 0; i <= segments; i++) {
        for (int j = 0; j <= segments; j++) {
            float pos1 = m_planeStart + i * dx1;
            float pos2 = m_planeStart2 + j * dx2;

            Vertex vertex;

            // Set position based on the selected axis
            switch (m_planeAxis) {
            case 0: // X-axis (YZ plane)
                vertex.position = glm::vec3(0.0f, pos1, pos2);
                break;
            case 1: // Y-axis (XZ plane)
                vertex.position = glm::vec3(pos1, 0.0f, pos2);
                break;
            case 2: // Z-axis (XY plane)
                vertex.position = glm::vec3(pos1, pos2, 0.0f);
                break;
            }

            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
            newMesh.vertices.push_back(vertex);
        }
    }

    // Generate indices for triangles
    for (int i = 0; i < segments; i++) {
        for (int j = 0; j < segments; j++) {
            int rowLength = segments + 1;
            int topLeft = i * rowLength + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * rowLength + j;
            int bottomRight = bottomLeft + 1;

            // First triangle (top-left, bottom-left, bottom-right)
            newMesh.indices.push_back(topLeft);
            newMesh.indices.push_back(bottomLeft);
            newMesh.indices.push_back(bottomRight);

            // Second triangle (top-left, bottom-right, top-right)
            newMesh.indices.push_back(topLeft);
            newMesh.indices.push_back(bottomRight);
            newMesh.indices.push_back(topRight);
        }
    }

    // Calculate number of triangles
    newMesh.numTriangles = newMesh.indices.size() / 3;

    // Calculate dimensions
    newMesh.CalculateDimensions();

    // Calculate memory usage
    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    // Add the mesh to your scene collection
    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));

    // Update scene collection
    renderer->setupSceneCollection();
}

void Application::createCubeMesh(std::string& meshName, const glm::vec3& center, const glm::vec3& dimensions, int LOD) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    // Define the 8 vertices of the cube (centered at (0, 0, 0) before scaling)
    glm::vec3 halfDimensions = dimensions * 0.5f;

    std::vector<glm::vec3> vertices = {
        glm::vec3(-halfDimensions.x, -halfDimensions.y, -halfDimensions.z), // 0
        glm::vec3(halfDimensions.x, -halfDimensions.y, -halfDimensions.z), // 1
        glm::vec3(halfDimensions.x,  halfDimensions.y, -halfDimensions.z), // 2
        glm::vec3(-halfDimensions.x,  halfDimensions.y, -halfDimensions.z), // 3
        glm::vec3(-halfDimensions.x, -halfDimensions.y,  halfDimensions.z), // 4
        glm::vec3(halfDimensions.x, -halfDimensions.y,  halfDimensions.z), // 5
        glm::vec3(halfDimensions.x,  halfDimensions.y,  halfDimensions.z), // 6
        glm::vec3(-halfDimensions.x,  halfDimensions.y,  halfDimensions.z)  // 7
    };

    // Apply the center translation
    for (auto& vertex : vertices) {
        vertex += center;
    }

    // Function to generate subdivided vertices for each face of the cube
    auto generateFaceVertices = [&](int faceIndex, glm::vec3 direction) {
        std::vector<glm::vec3> faceVertices;
        float step = 1.0f / LOD;
        for (int i = 0; i <= LOD; i++) {
            for (int j = 0; j <= LOD; j++) {
                glm::vec3 position;
                switch (faceIndex) {
                case 0: // front face (z = max)
                    position = glm::vec3(-halfDimensions.x + step * i * dimensions.x, -halfDimensions.y + step * j * dimensions.y, halfDimensions.z);
                    break;
                case 1: // back face (z = min)
                    position = glm::vec3(-halfDimensions.x + step * i * dimensions.x, -halfDimensions.y + step * j * dimensions.y, -halfDimensions.z);
                    break;
                case 2: // left face (x = min)
                    position = glm::vec3(-halfDimensions.x, -halfDimensions.y + step * i * dimensions.y, -halfDimensions.z + step * j * dimensions.z);
                    break;
                case 3: // right face (x = max)
                    position = glm::vec3(halfDimensions.x, -halfDimensions.y + step * i * dimensions.y, -halfDimensions.z + step * j * dimensions.z);
                    break;
                case 4: // bottom face (y = min)
                    position = glm::vec3(-halfDimensions.x + step * i * dimensions.x, -halfDimensions.y, -halfDimensions.z + step * j * dimensions.z);
                    break;
                case 5: // top face (y = max)
                    position = glm::vec3(-halfDimensions.x + step * i * dimensions.x, halfDimensions.y, -halfDimensions.z + step * j * dimensions.z);
                    break;
                }
                faceVertices.push_back(position + center);
            }
        }
        return faceVertices;
        };

    // Generate vertices for each face
    std::vector<std::vector<glm::vec3>> facesVertices;
    for (int i = 0; i < 6; i++) {
        facesVertices.push_back(generateFaceVertices(i, glm::vec3(0, 0, 0)));
    }

    // Add vertices to the mesh
    for (const auto& faceVertices : facesVertices) {
        for (const auto& vertexPos : faceVertices) {
            Vertex vertex;
            vertex.position = vertexPos;
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);  // Color can be adjusted
            newMesh.vertices.push_back(vertex);
        }
    }

    // Generate indices for the cube faces
    for (int i = 0; i < 6; i++) {
        int rowLength = LOD + 1;
        for (int j = 0; j < LOD; j++) {
            for (int k = 0; k < LOD; k++) {
                // Define quad by indices
                int topLeft = j * rowLength + k + i * (rowLength * (LOD + 1));
                int topRight = topLeft + 1;
                int bottomLeft = (j + 1) * rowLength + k + i * (rowLength * (LOD + 1));
                int bottomRight = bottomLeft + 1;

                // First triangle (top-left, bottom-left, bottom-right)
                newMesh.indices.push_back(topLeft);
                newMesh.indices.push_back(bottomLeft);
                newMesh.indices.push_back(bottomRight);

                // Second triangle (top-left, bottom-right, top-right)
                newMesh.indices.push_back(topLeft);
                newMesh.indices.push_back(bottomRight);
                newMesh.indices.push_back(topRight);
            }
        }
    }

    // Calculate the number of triangles
    newMesh.numTriangles = newMesh.indices.size() / 3;

    // Calculate dimensions
    newMesh.CalculateDimensions();

    // Calculate memory usage
    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    // Add the mesh to your scene collection
    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));

    // Update scene collection
    renderer->setupSceneCollection();
}

void Application::createSphereMesh(std::string& meshName, glm::vec3& center, float radius, int LOD) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    // Number of segments (more segments means higher detail)
    int segments = LOD;  // The level of detail determines the number of segments
    float phiStep = glm::pi<float>() / segments;  // Latitude angle step
    float thetaStep = glm::pi<float>() * 2.0f / segments;  // Longitude angle step

    // Generate vertices
    for (int i = 0; i <= segments; ++i) {
        float phi = i * phiStep;
        for (int j = 0; j <= segments; ++j) {
            float theta = j * thetaStep;

            // Spherical to Cartesian conversion
            float x = center.x + radius * sin(phi) * cos(theta);
            float y = center.y + radius * cos(phi);
            float z = center.z + radius * sin(phi) * sin(theta);

            Vertex vertex;
            vertex.position = glm::vec3(x, y, z);
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);  // White color for the vertices
            newMesh.vertices.push_back(vertex);
        }
    }

    // Generate indices for the sphere (connecting the vertices into triangles)
    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;

            // First triangle (top-left, bottom-left, bottom-right)
            newMesh.indices.push_back(first);
            newMesh.indices.push_back(second);
            newMesh.indices.push_back(first + 1);

            // Second triangle (top-left, bottom-right, top-right)
            newMesh.indices.push_back(second);
            newMesh.indices.push_back(second + 1);
            newMesh.indices.push_back(first + 1);
        }
    }

    // Calculate number of triangles
    newMesh.numTriangles = newMesh.indices.size() / 3;

    // Calculate dimensions
    newMesh.CalculateDimensions();

    // Calculate memory usage
    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    // Add the mesh to your scene collection
    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));

    // Update scene collection
    renderer->setupSceneCollection();
}

void Application::createCylinderMesh(std::string& meshName, glm::vec3& center, float radius, float height, int LOD) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    int segments = LOD;
    float angleStep = glm::two_pi<float>() / segments;
    float halfHeight = height / 2.0f;

    // Generate vertices
    for (int i = 0; i < segments; i++) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        Vertex topVertex;
        topVertex.position = glm::vec3(x, halfHeight, z) + center;  // Top circle
        topVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        newMesh.vertices.push_back(topVertex);

        Vertex bottomVertex;
        bottomVertex.position = glm::vec3(x, -halfHeight, z) + center;  // Bottom circle
        bottomVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        newMesh.vertices.push_back(bottomVertex);
    }

    // Add top and bottom center vertices
    Vertex topCenterVertex;
    topCenterVertex.position = glm::vec3(0.0f, halfHeight, 0.0f) + center;
    topCenterVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
    newMesh.vertices.push_back(topCenterVertex);

    Vertex bottomCenterVertex;
    bottomCenterVertex.position = glm::vec3(0.0f, -halfHeight, 0.0f) + center;
    bottomCenterVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
    newMesh.vertices.push_back(bottomCenterVertex);

    // Generate indices for the cylinder side
    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;

        // Side triangles
        newMesh.indices.push_back(i * 2);
        newMesh.indices.push_back(next * 2);
        newMesh.indices.push_back(i * 2 + 1);

        newMesh.indices.push_back(next * 2);
        newMesh.indices.push_back(next * 2 + 1);
        newMesh.indices.push_back(i * 2 + 1);
    }

    // Generate indices for the top and bottom caps
    int topCenterIndex = 2 * segments;
    int bottomCenterIndex = 2 * segments + 1;

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;

        // Top cap
        newMesh.indices.push_back(topCenterIndex);
        newMesh.indices.push_back(i * 2);
        newMesh.indices.push_back(next * 2);

        // Bottom cap
        newMesh.indices.push_back(bottomCenterIndex);
        newMesh.indices.push_back(next * 2 + 1);
        newMesh.indices.push_back(i * 2 + 1);
    }

    // Calculate number of triangles
    newMesh.numTriangles = newMesh.indices.size() / 3;

    // Calculate dimensions
    newMesh.CalculateDimensions();

    // Calculate memory usage
    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    // Add the mesh to your scene collection
    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));

    // Update scene collection
    renderer->setupSceneCollection();
}

void Application::createDiskMesh(std::string& meshName, glm::vec3& center, float radius, int LOD, int axis) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    int segments = LOD;
    float angleStep = glm::two_pi<float>() / segments;

    // Add center vertex
    Vertex centerVertex;
    centerVertex.position = center;
    centerVertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
    newMesh.vertices.push_back(centerVertex);

    // Generate perimeter vertices based on axis
    for (int i = 0; i < segments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        Vertex vertex;

        switch (axis) {
            case 0: // X-axis (disk lies in YZ plane)
                vertex.position = glm::vec3(center.x, center.y + x, center.z + y);
                break;
            case 1: // Y-axis (disk lies in XZ plane)
                vertex.position = glm::vec3(center.x + x, center.y, center.z + y);
                break;
            case 2: // Z-axis (disk lies in XY plane)
            default:
                vertex.position = glm::vec3(center.x + x, center.y + y, center.z);
                break;
        }

        vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        newMesh.vertices.push_back(vertex);
    }

    // Generate triangle indices (triangle fan)
    int centerIndex = 0;
    for (int i = 1; i < segments; ++i) {
        newMesh.indices.push_back(centerIndex);
        newMesh.indices.push_back(i);
        newMesh.indices.push_back(i + 1);
    }
    // Close the disk
    newMesh.indices.push_back(centerIndex);
    newMesh.indices.push_back(segments);
    newMesh.indices.push_back(1);

    newMesh.numTriangles = newMesh.indices.size() / 3;
    newMesh.CalculateDimensions();
    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));
    renderer->setupSceneCollection();
}

void Application::createTrihedralReflectorMesh(std::string& meshName, glm::vec3& center, float size) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    float h = size; // Half-length of each square face edge

    // Define the 3 perpendicular planes meeting at the center
    glm::vec3 p0 = center;                         // Corner point (common vertex)
    glm::vec3 p1 = center + glm::vec3(h, 0, 0);    // X direction
    glm::vec3 p2 = center + glm::vec3(0, h, 0);    // Y direction
    glm::vec3 p3 = center + glm::vec3(0, 0, h);    // Z direction

    glm::vec3 p4 = center + glm::vec3(h, h, 0);    // XY plane
    glm::vec3 p5 = center + glm::vec3(h, 0, h);    // XZ plane
    glm::vec3 p6 = center + glm::vec3(0, h, h);    // YZ plane

    // Set color white
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    // Vertices
    std::vector<glm::vec3> positions = { p0, p1, p2, p3, p4, p5, p6 };
    for (const auto& pos : positions) {
        Vertex v;
        v.position = pos;
        v.color = color;
        newMesh.vertices.push_back(v);
    }

    // Indices for the three triangle faces
    // Each triangle is defined as (corner, edge1, edge2)
    newMesh.indices.insert(newMesh.indices.end(), {
        0, 1, 2,  // XY plane
        0, 1, 3,  // XZ plane
        0, 2, 3   // YZ plane
        });

    newMesh.numTriangles = newMesh.indices.size() / 3;
    newMesh.CalculateDimensions();

    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));
    renderer->setupSceneCollection();
}

void Application::createDihedralReflectorMesh(std::string& meshName, glm::vec3& center, float size) {
    Mesh newMesh;
    newMesh.fileName = meshName;

    float h = size; // Half-length of square face edge

    // Define base point (corner where plates meet)
    glm::vec3 p0 = center;

    // First plane in XY
    glm::vec3 p1 = p0 + glm::vec3(h, 0, 0);
    glm::vec3 p2 = p0 + glm::vec3(0, h, 0);
    glm::vec3 p3 = p0 + glm::vec3(h, h, 0);

    // Second plane in XZ
    glm::vec3 p4 = p0 + glm::vec3(0, 0, h);
    glm::vec3 p5 = p0 + glm::vec3(h, 0, h);

    glm::vec3 color(1.0f, 1.0f, 1.0f); // white color

    // Push vertices
    std::vector<glm::vec3> positions = { p0, p1, p2, p3, p4, p5 };
    for (const auto& pos : positions) {
        Vertex v;
        v.position = pos;
        v.color = color;
        newMesh.vertices.push_back(v);
    }

    // Indices for two square planes, each as two triangles
    // XY face: p0, p1, p2, p3
    newMesh.indices.insert(newMesh.indices.end(), {
        0, 1, 2,
        2, 1, 3
        });

    // XZ face: p0, p1, p4, p5
    newMesh.indices.insert(newMesh.indices.end(), {
        0, 1, 4,
        4, 1, 5
        });

    newMesh.numTriangles = newMesh.indices.size() / 3;
    newMesh.CalculateDimensions();

    newMesh.modelMemoryMB = (newMesh.vertices.size() * sizeof(Vertex) + newMesh.indices.size() * sizeof(GLuint)) / (1024.0f * 1024.0f);

    renderer->sceneCollectionMeshes.push_back(std::move(newMesh));
    renderer->setupSceneCollection();
}

float Application::GetCPUutilization()
{
    static ULARGE_INTEGER lastIdleTime = {};
    static ULARGE_INTEGER lastKernelTime = {};
    static ULARGE_INTEGER lastUserTime = {};

    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return -1.0f;
    }

    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;

    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;

    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    ULONGLONG sysIdle = idle.QuadPart - lastIdleTime.QuadPart;
    ULONGLONG sysKernel = kernel.QuadPart - lastKernelTime.QuadPart;
    ULONGLONG sysUser = user.QuadPart - lastUserTime.QuadPart;
    ULONGLONG sysTotal = sysKernel + sysUser;

    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;

    if (sysTotal == 0) return 0.0f;

    return (1.0f - ((float)sysIdle / sysTotal)) * 100.0f;
}

std::string Application::GetGPUutilization() {
    nvmlInit();  // Initialize NVML

    nvmlDevice_t device;
    nvmlMemory_t memoryInfo;

    // Stringstream to build the result string
    std::ostringstream gpuInfo;

    // Get the handle for the first GPU (since you have only one GPU)
    nvmlDeviceGetHandleByIndex(0, &device);

    // Get the memory info for the device (VRAM)
    nvmlDeviceGetMemoryInfo(device, &memoryInfo);

    // Calculate the percentage of used memory
    float usedGB = static_cast<float>(memoryInfo.used) / (1024.0f * 1024.0f * 1024.0f);
    float totalGB = static_cast<float>(memoryInfo.total) / (1024.0f * 1024.0f * 1024.0f);
    float percent = (usedGB / totalGB) * 100.0f;

    // Add the formatted string to the stringstream
    gpuInfo << std::fixed << std::setprecision(1)
        << usedGB << " GB / "  // Used memory in GB
        << totalGB << " GB ("  // Total memory in GB
        << static_cast<int>(percent) << "%)";  // Percentage used

    nvmlShutdown();  // Shutdown NVML

    return gpuInfo.str();  // Return the formatted string
}

std::string Application::GetRAMutilization()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    DWORDLONG usedPhysMem = totalPhysMem - availPhysMem;

    float usedGB = static_cast<float>(usedPhysMem) / (1024.0f * 1024.0f * 1024.0f);
    float totalGB = static_cast<float>(totalPhysMem) / (1024.0f * 1024.0f * 1024.0f);
    float percent = (usedGB / totalGB) * 100.0f;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1)
        << usedGB << " GB / " << totalGB << " GB (" << static_cast<int>(percent) << "%)";

    return oss.str();
}

void Application::Init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(-1);
    }

    // Set GLFW window hints for OpenGL version and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Start maximized
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window
    window = glfwCreateWindow(1280, 720, "ScatterX", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        std::exit(-1);
    }
    glfwSetMouseButtonCallback(window, renderer->m_InputManager.MouseButtonCallback);

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        std::exit(-1);
    }

    // Load renderer class
    renderer = std::make_unique<Renderer>();

    int iconWidth, iconHeight;
    folderIconTextureID = LoadTextureFromFile("assets/folder_icon.png", &iconWidth, &iconHeight);
    undoButtonTextureID = LoadTextureFromFile("assets/undo_button.png", &iconWidth, &iconHeight);
    objIconTextureID = LoadTextureFromFile("assets/obj_icon.png", &iconWidth, &iconHeight);
    coordinateSystemTextureID = LoadTextureFromFile("assets/coordinate_system.png", &iconWidth, &iconHeight);
    resetIconTextureID = LoadTextureFromFile("assets/reset_icon.png", &iconWidth, &iconHeight);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Set the global font scale
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.3f;

    // Setup ImGui backend bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
}

void Application::drawPerformanceMetrics(float fps, float cpuUsage, std::string ramUsage, std::string gpuUsage)
{
    // Set the window position to a little below the top-left corner of the screen
    ImVec2 windowPos(10, 50);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

    // Set window size explicitly and ensure it won't change
    ImVec2 windowSize(300, 120);  // Width: 220, Height: 120
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    // Set window flags to make it non-movable and non-resizable
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;

    // Push the custom color for the title background (active and inactive)
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    // Create the window
    ImGui::Begin("Performance Metrics", nullptr, windowFlags);

    // Optional: Make the font smaller for the panel text
    ImGui::Text("FPS: %.2f", fps);
    ImGui::Text("CPU: %.2f%%", cpuUsage);
    ImGui::Text("RAM: %s", ramUsage.c_str());
    ImGui::Text("VRAM: %s", gpuUsage.c_str());

    // End the window
    ImGui::End();

    // Pop the style color settings to restore default
    ImGui::PopStyleColor(3); 
}

void Application::drawContentBrowser()
{
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    float screenWidth = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    float screenHeight = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
    float contentBrowserHeight = screenHeight * 0.3f; // 30% of screen height
    float contentBrowserY = screenHeight - contentBrowserHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, contentBrowserY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth * 0.7f - 300, contentBrowserHeight), ImGuiCond_Always); // 70% width for Content Browser
    ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImVec2 iconSize(64, 64);  // Size of each icon
    float padding = 10.0f;
    float itemWidth = iconSize.x + padding * 2;  // Width of each item including padding
    float itemHeight = iconSize.y + padding * 2 + ImGui::GetTextLineHeightWithSpacing(); // Height including padding and text

    float windowWidth = ImGui::GetContentRegionAvail().x;
    int itemsPerRow = static_cast<int>(windowWidth / itemWidth);
    itemsPerRow = std::max(1, itemsPerRow); // Ensure at least 1 item per row

    // Apply left padding to the whole content area
    ImGui::Indent(padding);

    // Back button if we're not in the root directory
    if (!isInRootDirectory) {
        ImVec2 startPos = ImGui::GetCursorPos();

        // Make the back button smaller (40x40 instead of 64x64)
        ImVec2 backButtonSize(40, 40);

        // Make the back button clickable with the objIconTextureID
        ImGui::PushID("BackButton");
        bool backClicked = ImGui::InvisibleButton("##back", backButtonSize);

        // Reset cursor position to draw on top of the button
        ImGui::SetCursorPos(startPos);

        // Display the objIconTextureID as black button (remove text, just show icon)
        ImGui::Image(undoButtonTextureID, backButtonSize, ImVec2(0, 0), ImVec2(1, 1),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0, 0, 0, 0));

        // Handle hover effect
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Return to root directory");
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 topLeft = ImGui::GetItemRectMin();
            ImVec2 bottomRight = ImGui::GetItemRectMax();
            drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(100, 100, 100, 100), 5.0f);
        }

        ImGui::PopID();

        // Handle click
        if (backClicked) {
            contentBrowserPath = "./Database";
            isInRootDirectory = true;
        }

        // Add extra space after the back button
        ImGui::Dummy(ImVec2(0.0f, padding * 2));
    }

    if (isInRootDirectory) {
        // Display root folders
        std::vector<std::string> folders;
        for (const auto& entry : std::filesystem::directory_iterator(contentBrowserPath)) {
            if (entry.is_directory()) {
                folders.push_back(entry.path().filename().string());
            }
        }

        for (size_t i = 0; i < folders.size(); i++) {
            // Calculate position for grid layout
            if (i % itemsPerRow != 0) {
                ImGui::SameLine(0.0f, padding); // Add spacing between items
            }
            else if (i > 0) {
                // Add vertical spacing between rows
                ImGui::Dummy(ImVec2(0.0f, padding));
            }

            // Generate a unique ID for the clickable area
            ImGui::PushID(static_cast<int>(i));

            // Create the clickable area
            ImGui::BeginGroup();

            // Store current cursor position
            ImVec2 startPos = ImGui::GetCursorPos();

            // Make the entire group clickable
            bool isClicked = ImGui::InvisibleButton("##folder", ImVec2(iconSize.x, iconSize.y + ImGui::GetTextLineHeightWithSpacing()));

            // Reset cursor position to draw on top of the button
            ImGui::SetCursorPos(startPos);

            // Display the folder icon
            ImGui::Image(folderIconTextureID, iconSize);

            // Display folder name centered under the icon
            float textWidth = ImGui::CalcTextSize(folders[i].c_str()).x;
            float centerOffset = (iconSize.x - textWidth) * 0.5f;
            centerOffset = std::max(0.0f, centerOffset); // Prevent negative offset
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);
            ImGui::Text("%s", folders[i].c_str());

            ImGui::EndGroup();

            // Handle click
            if (isClicked) {
                contentBrowserPath = "./Database/" + folders[i];
                isInRootDirectory = false;
            }

            // Add hover effect
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Open %s folder", folders[i].c_str());
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 topLeft = ImGui::GetItemRectMin();
                ImVec2 bottomRight = ImGui::GetItemRectMax();
                drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(200, 200, 200, 50), 5.0f);
            }

            ImGui::PopID();
        }
    }
    else {
        // Display .obj files in the selected folder
        std::vector<std::string> objFiles;
        for (const auto& entry : std::filesystem::directory_iterator(contentBrowserPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".obj") {
                objFiles.push_back(entry.path().filename().string());
            }
        }

        for (size_t i = 0; i < objFiles.size(); i++) {
            // Calculate position for grid layout
            if (i % itemsPerRow != 0) {
                ImGui::SameLine(0.0f, padding); // Add spacing between items
            }
            else if (i > 0) {
                // Add vertical spacing between rows
                ImGui::Dummy(ImVec2(0.0f, padding));
            }

            // Generate a unique ID for the clickable area
            ImGui::PushID(static_cast<int>(i));

            // Create the clickable area
            ImGui::BeginGroup();

            // Store current cursor position
            ImVec2 startPos = ImGui::GetCursorPos();

            // Make the entire group clickable
            bool isClicked = ImGui::InvisibleButton("##obj", ImVec2(iconSize.x, iconSize.y + ImGui::GetTextLineHeightWithSpacing()));

            // Reset cursor position to draw on top of the button
            ImGui::SetCursorPos(startPos);

            // Display the .obj icon
            ImGui::Image(objIconTextureID, iconSize);

            // Display the .obj filename (without extension)
            //std::string fileNameWithoutExtension = objFiles[i];
            //fileNameWithoutExtension = fileNameWithoutExtension.substr(0, fileNameWithoutExtension.find_last_of('.'));  // Remove the .obj extension
            float textWidth = ImGui::CalcTextSize(objFiles[i].c_str()).x;
            float centerOffset = (iconSize.x - textWidth) * 0.5f;
            centerOffset = std::max(0.0f, centerOffset); // Prevent negative offset
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);
            ImGui::Text("%s", objFiles[i].c_str());

            ImGui::EndGroup();

            // Handle click
            if (isClicked) {
                m_showMeshOptions = false;
                m_showSceneOptions = false;
                // Handle file selection
                selectedItemPathContentBrowser = contentBrowserPath + "/" + objFiles[i];
            }

            // Add hover effect
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Load %s", objFiles[i].c_str());
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 topLeft = ImGui::GetItemRectMin();
                ImVec2 bottomRight = ImGui::GetItemRectMax();
                drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(200, 200, 200, 50), 5.0f);
            }

            ImGui::PopID();
        }
    }

    // Remove the indent we added
    ImGui::Unindent(padding);

    ImGui::End();
    ImGui::PopStyleColor(4); // Pop styles
}

void Application::drawResultsPanel()
{
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    float screenWidth = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    float screenHeight = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

    float contentBrowserHeight = screenHeight * 0.3f; // 30% of screen height
    float contentBrowserY = screenHeight - contentBrowserHeight;

    // Set next window size for the Results panel (30% of width)
    ImGui::SetNextWindowPos(ImVec2(screenWidth * 0.7f-300, contentBrowserY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth * 0.3f, contentBrowserHeight), ImGuiCond_Always); // 30% width for the right panel

    ImGui::Begin("Results", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::End();
    ImGui::PopStyleColor(4); // Pop styles
}

void Application::drawSceneCollection()
{
    // Set title background and text to static 
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));  // Background of the panel titlebar
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));  // Background of the panel titlebar when not active
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));  // Title text color
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Content area color

    ImGui::SetNextWindowPos(ImVec2((float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width - 300.0f, 31.0f), ImGuiCond_Always); // Position at right top
    ImGui::SetNextWindowSize(ImVec2(300.0f, 0.25f * glfwGetVideoMode(glfwGetPrimaryMonitor())->height), ImGuiCond_Always); // Take 40% of the screen height
    ImGui::Begin("Scene Collection", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Scene Collection content with clickable sections
    if (ImGui::Selectable("Camera & Scene", selectedObjectNameSceneCollection == "Camera & Scene")) {
        if (selectedObjectNameSceneCollection == "Camera & Scene") selectedObjectNameSceneCollection = "";
        else 
        {
            selectedObjectNameSceneCollection = "Camera & Scene";
            m_showMeshOptions = false;
            m_showSceneOptions = false;
        }
    }
    else {
        // Display all objects from sceneCollectionMeshes
        if (renderer) {  // Make sure renderer is initialized
            for (size_t i = 0; i < renderer->sceneCollectionMeshes.size(); i++) {
                const std::string& meshName = renderer->sceneCollectionMeshes[i].fileName;

                // Check if this mesh is currently selected
                bool isSelected = (selectedObjectNameSceneCollection == meshName);

                // Display selectable item for each mesh
                if (ImGui::Selectable(meshName.c_str(), isSelected)) {
                    if (isSelected) {
                        // If already selected, deselect it
                        selectedObjectNameSceneCollection = "";
                    }
                    else {
                        m_showMeshOptions = false;
                        m_showSceneOptions = false;
                        // Otherwise, select this object
                        selectedObjectNameSceneCollection = meshName;
                    }
                }
            }
        }
    }

    // You can add more selectable sections here

    ImGui::End(); // End Scene Collection panel

    // Pop style for Scene Collection
    ImGui::PopStyleColor(4);  // Pop the 4 styles for Scene Collection window
}

void Application::drawSceneInspector()
{
    // Place it 150 pixels above bottom-left
    ImVec2 windowPos(10, 350);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

    ImVec2 windowSize(300, 120);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    size_t totalTriangles = 0;
    float totalMemoryMB = 0.0f;

    for (const auto& mesh : renderer->sceneCollectionMeshes)
    {
        totalTriangles += mesh.numTriangles;
        totalMemoryMB += mesh.modelMemoryMB;
    }

    ImGui::Begin("Scene Inspector", nullptr, windowFlags);

    ImGui::Text("Objects in Scene: %zu", renderer->sceneCollectionMeshes.size());
    ImGui::Text("Total Triangles: %zu", totalTriangles);
    ImGui::Text("Total Memory: %.2f MB", totalMemoryMB);
    ImGui::Text("Grid Step Size: %.3f m", renderer->step);

    ImGui::End();

    ImGui::PopStyleColor(3);
}

void Application::drawObjectEditor()
{
    // Set title background and text to static
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    ImGui::SetNextWindowPos(ImVec2((float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width - 300.0f, 0.25f * glfwGetVideoMode(glfwGetPrimaryMonitor())->height + 31), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 0.75f * glfwGetVideoMode(glfwGetPrimaryMonitor())->height), ImGuiCond_Always);

    ImGui::Begin("Object Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (selectedObjectNameSceneCollection == "Camera & Scene") {
        // Camera settings
        // Add space at the top
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        // Make title bold and pure white
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Center the text
        const char* title = "Camera Settings";
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextWrapped("%s", title);

        ImGui::PopStyleColor();


        // Add space below the title
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // Optional: Draw a thin separator line
        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 8.0f)); // Space after separator

        ImGui::PushItemWidth(175.0f);

        // Darker slider style
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        ImGui::SliderFloat("FOV", &renderer->camera.fov, 15.0f, 120.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Near Plane", &renderer->camera.nearPlane, 0.01f, 0.1f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Far Plane", &renderer->camera.farPlane, 100.0f, 10000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Speed", &renderer->camera.speed, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Sensitivity", &renderer->camera.sensitivity, 75.0f, 200.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::PopStyleColor(3);
        ImGui::PopItemWidth();

        // Calculate the width of the button
        float buttonWidth = ImGui::GetContentRegionAvail().x * 0.6f; // 60% of available width
        float buttonX = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f; // Center the button

        // Add some spacing
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // Center the button
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonX);

        // Push the button styling
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));          // Gray 
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));   // Lighter gray on hover 
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); // Darker gray when pressed

        // Create the Reset button
        if (ImGui::Button("Reset Camera", ImVec2(buttonWidth, 0))) {
            m_showMeshOptions = false;
            m_showSceneOptions = false;
            // Reset camera values to defaults
           renderer->camera.fov = 45.0f;
           renderer->camera.nearPlane = 0.1f;
           renderer->camera.farPlane = 100.0f;
           renderer->camera.speed = 0.2f;
           renderer->camera.sensitivity = 100.0f;
        }

        // Pop the button styling
        ImGui::PopStyleColor(3);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // Scene Settings
        title = "Scene Settings";
        windowWidth = ImGui::GetContentRegionAvail().x;
        textWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextWrapped("%s", title);

        // Add space below the title
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // Optional: Draw a thin separator line
        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 8.0f)); // Space after separator

        ImGui::PushItemWidth(175.0f);

        // Darker slider style
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

        if (ImGui::SliderFloat("Grid Size", &renderer->size, 10.0f, 500.0f, "%.0f")) {
            renderer->gridNeedsUpdate = true;
        }
        if (ImGui::SliderFloat("Division", &renderer->divisions, 10.0f, 500.0f, "%.0f")) {
            renderer->gridNeedsUpdate = true;
        }

        ImGui::PopStyleColor(3);
        ImGui::PopItemWidth();

        buttonWidth = ImGui::GetContentRegionAvail().x * 0.6f; // 60% of available width
        buttonX = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f; // Center the button

        // Add some spacing
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        // Center the button
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonX);

        // Push the button styling
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));          // Gray 
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));   // Lighter gray on hover 
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); // Darker gray when pressed

        // Create the Reset button
        if (ImGui::Button("Reset Scene", ImVec2(buttonWidth, 0))) {
            m_showMeshOptions = false;
            m_showSceneOptions = false;
            // Reset grid values to defaults
            renderer->size = 150.0f;
            renderer->divisions = 150.0f;
            renderer->gridNeedsUpdate = true;
        }

        // Pop the button styling
        ImGui::PopStyleColor(3);
    }
    else if (selectedObjectNameSceneCollection != "") {
        // Check if the selected object has .obj extension
        std::string extension = selectedObjectNameSceneCollection.substr(
            selectedObjectNameSceneCollection.find_last_of(".") != std::string::npos ?
            selectedObjectNameSceneCollection.find_last_of(".") : selectedObjectNameSceneCollection.length());

        if (extension == ".obj" || extension == ".mesh") {
            // Find the selected mesh in the sceneCollectionMeshes vector
            int selectedMeshIndex = -1;
            for (size_t i = 0; i < renderer->sceneCollectionMeshes.size(); i++) {
                if (renderer->sceneCollectionMeshes[i].fileName == selectedObjectNameSceneCollection) {
                    selectedMeshIndex = static_cast<int>(i);
                    break;
                }
            }

            if (selectedMeshIndex != -1) {
                // Add a little space at the top
                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                // Make the object settings title bold and white
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text

                std::string title = selectedObjectNameSceneCollection + " Settings";
                float textWidth = ImGui::CalcTextSize(title.c_str()).x;
                float panelWidth = ImGui::GetWindowSize().x;
                ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
                ImGui::TextWrapped("%s", title.c_str());

                ImGui::PopStyleColor();


                // Space below title
                ImGui::Dummy(ImVec2(0.0f, 8.0f));

                // Draw a clean separator line
                ImGui::Separator();

                // Space after separator
                ImGui::Dummy(ImVec2(0.0f, 8.0f));

                // Center-align "Number of Triangles"
                {
                    std::string triText = std::format("Number of Triangles: {}", renderer->sceneCollectionMeshes[selectedMeshIndex].numTriangles);
                    float textWidth = ImGui::CalcTextSize(triText.c_str()).x;
                    float panelWidth = ImGui::GetWindowSize().x;
                    ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
                    ImGui::Text("%s", triText.c_str());
                }

                // Center-align "Memory Allocation"
                {
                    std::string memText = std::format("Memory Allocation: {:.2f} MB", renderer->sceneCollectionMeshes[selectedMeshIndex].modelMemoryMB);
                    float textWidth = ImGui::CalcTextSize(memText.c_str()).x;
                    float panelWidth = ImGui::GetWindowSize().x;
                    ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
                    ImGui::Text("%s", memText.c_str());
                }

                ImGui::Spacing();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Slider styles
                ImGui::PushItemWidth(175.0f);
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

                // Location sliders with improved alignment
                ImGui::Text("Location");
                ImGui::Indent(50);

                // X axis (red)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
                ImGui::Text("X");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##LocX", &renderer->sceneCollectionMeshes[selectedMeshIndex].position.x, -renderer->size / 2, renderer->size / 2, "%.1fm")) {
                    // Call UpdateModelMatrix whenever a transform changes
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetLocX", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].position.x = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Y axis (green)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 255, 100, 255));
                ImGui::Text("Y");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##LocY", &renderer->sceneCollectionMeshes[selectedMeshIndex].position.y, -renderer->size / 2, renderer->size / 2, "%.1fm")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetLocY", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].position.y = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Z axis (blue)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 255, 255));
                ImGui::Text("Z");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##LocZ", &renderer->sceneCollectionMeshes[selectedMeshIndex].position.z, -renderer->size / 2, renderer->size / 2, "%.1fm")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetLocZ", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].position.z = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                ImGui::Unindent(50);
                ImGui::Spacing();

                // Rotation sliders with improved alignment
                ImGui::Text("Rotation");
                ImGui::Indent(50);

                // X axis (red)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
                ImGui::Text("X");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##RotX", &renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.x, 0.0f, 360.0f, "%.0fdeg")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetRotX", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.x = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Y axis (green)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 255, 100, 255));
                ImGui::Text("Y");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##RotY", &renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.y, 0.0f, 360.0f, "%.0fdeg")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetRotY", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.y = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Z axis (blue)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 255, 255));
                ImGui::Text("Z");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##RotZ", &renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.z, 0.0f, 360.0f, "%.0fdeg")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetRotZ", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].rotation.z = 0.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                ImGui::Unindent(50);
                ImGui::Spacing();

                // Scale sliders with improved alignment
                ImGui::Text("Scale");
                ImGui::Indent(50);

                // X axis (red)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
                ImGui::Text("X");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##ScaleX", &renderer->sceneCollectionMeshes[selectedMeshIndex].scale.x, 0.1f, 10.0f, "%.1f")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetScaleX", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].scale.x = 1.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Y axis (green)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 255, 100, 255));
                ImGui::Text("Y");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##ScaleY", &renderer->sceneCollectionMeshes[selectedMeshIndex].scale.y, 0.1f, 10.0f, "%.1f")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetScaleY", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].scale.y = 1.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                // Z axis (blue)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 255, 255));
                ImGui::Text("Z");
                ImGui::PopStyleColor();

                ImGui::SameLine(75);
                if (ImGui::SliderFloat("##ScaleZ", &renderer->sceneCollectionMeshes[selectedMeshIndex].scale.z, 0.1f, 10.0f, "%.1f")) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }
                ImGui::SameLine();
                if (ImGui::ImageButton("##ResetScaleZ", (ImTextureID)(intptr_t)this->resetIconTextureID, ImVec2(15, 15))) {
                    renderer->sceneCollectionMeshes[selectedMeshIndex].scale.z = 1.0f;
                    renderer->sceneCollectionMeshes[selectedMeshIndex].UpdateModelMatrix();
                }

                ImGui::Unindent(50);
                ImGui::PopStyleColor(3); // Pop slider style colors
                ImGui::PopItemWidth();

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();

                // Set button width and center it
                float windowWidth = ImGui::GetWindowSize().x;
                float buttonWidth = 250;
                ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

                // Set Geometry to Origin button - Gray themed
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));          // Gray
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));   // Lighter gray on hover
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); // Darker gray when pressed

                if (ImGui::Button("Set Geometry to Origin", ImVec2(buttonWidth, 30))) {
                    m_showMeshOptions = false;
                    m_showSceneOptions = false;
                    SetGeometryToOrigin(selectedMeshIndex);
                }

                ImGui::PopStyleColor(3); // Pop the button style colors

                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Separator();

                // Make the object settings title bold and white
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text

                // Center-align the "Length" text
                {
                    std::string lengthText = std::format("Length: {:.1f} meters", renderer->sceneCollectionMeshes[selectedMeshIndex].length);
                    float textWidth = ImGui::CalcTextSize(lengthText.c_str()).x;
                    float panelWidth = ImGui::GetWindowSize().x;
                    ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
                    ImGui::TextWrapped("%s", lengthText.c_str());
                }

                // Center-align the "Height" text
                {
                    std::string heightText = std::format("Height: {:.1f} meters", renderer->sceneCollectionMeshes[selectedMeshIndex].height);
                    float textWidth = ImGui::CalcTextSize(heightText.c_str()).x;
                    float panelWidth = ImGui::GetWindowSize().x;
                    ImGui::SetCursorPosX((panelWidth - textWidth) * 0.5f);
                    ImGui::TextWrapped("%s", heightText.c_str());
                }

                ImGui::PopStyleColor();
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0.0f, 90.0f));


                // Delete button - centered with dark theme
                windowWidth = ImGui::GetWindowSize().x;
                buttonWidth = 150;
                ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

                // Delete button - Dark themed red
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));          // Dark red
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));     // Brighter red on hover
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));      // Darker red pressed

                if (ImGui::Button("DELETE OBJ", ImVec2(buttonWidth, 30))) {
                    m_showMeshOptions = false;
                    m_showSceneOptions = false;
                    // Remove the selected mesh from the scene
                    if (selectedMeshIndex >= 0 && selectedMeshIndex < renderer->sceneCollectionMeshes.size()) {
                        renderer->sceneCollectionMeshes.erase(renderer->sceneCollectionMeshes.begin() + selectedMeshIndex);
                    }
                    renderer->setupSceneCollection();
                    selectedMeshIndex = -1;
                    selectedObjectNameSceneCollection = ""; // Clear selection after deletion
                }

                ImGui::PopStyleColor(3);
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(4);
}

void Application::renderObjectSelectionWindow()
{
    if (selectedItemPathContentBrowser.empty()) {
        return; // Don't show popup if no item is selected
    }

    // Calculate center of screen for popup position
    int screenWidth = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    int screenHeight = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
    ImVec2 center(screenWidth * 0.5f, screenHeight * 0.5f);

    // Set popup position and fixed size - don't use AutoResize
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400.0f, 250.0f));

    // Style for popup
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

    // Extract filename from path
    std::string fileName = selectedItemPathContentBrowser.substr(
        selectedItemPathContentBrowser.find_last_of("/\\") + 1);

    // Begin popup window with a title showing the filename
    if (ImGui::Begin(("Selected Object: " + fileName).c_str(), nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        static Mesh tempMesh;
        static bool meshLoaded = false;
        static int triangleCount = 0;
        static float meshSizeMB = 0.0f;
        static std::string lastSelectedPath = "";

        // Load mesh data if path has changed
        if (lastSelectedPath != selectedItemPathContentBrowser) {
            lastSelectedPath = selectedItemPathContentBrowser;
            meshLoaded = false;

            tempMesh = Mesh(selectedItemPathContentBrowser);

            triangleCount = tempMesh.numTriangles;
            meshSizeMB = tempMesh.modelMemoryMB;
            meshLoaded = true;
        }

        if (meshLoaded) {
            // Object info section
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            ImGui::TextWrapped("Information");
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::Spacing();

            // Display mesh information with fixed width to prevent expanding
            float contentWidth = ImGui::GetContentRegionAvail().x;
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + contentWidth);
            ImGui::Text("Filename: %s", fileName.c_str());
            ImGui::Text("Number of Triangles: %d", triangleCount);
            ImGui::Text("Size: %.2f MB", meshSizeMB);
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            ImGui::Spacing();

            // Create button area at the bottom of the popup
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
            ImGui::Separator();
            ImGui::Spacing();

            // Create a horizontal layout for buttons
            float windowWidth = ImGui::GetWindowSize().x;
            float buttonWidth = (windowWidth - 50) / 2.0f;

            // Load button with green theme
            ImGui::SetCursorPosX((windowWidth - (buttonWidth * 2 + 20)) / 2);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));

            if (ImGui::Button("Load", ImVec2(buttonWidth, 30))) {
                m_showMeshOptions = false;
                m_showSceneOptions = false;
                renderer->sceneCollectionMeshes.push_back(std::move(tempMesh));
                renderer->setupSceneCollection();
                selectedItemPathContentBrowser = ""; // Close popup
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 20);

            // Cancel/Delete button with red theme
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 30))) {
                m_showMeshOptions = false;
                m_showSceneOptions = false;
                selectedItemPathContentBrowser = ""; // Close popup
                tempMesh.Clean();
                renderer->setupSceneCollection();
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::End();
    }

    ImGui::PopStyleColor(4);
}

void Application::drawTitleBar()
{
    // Title bar
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always); // Position at top-left
    ImGui::SetNextWindowSize(ImVec2((float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width, 30.0f), ImGuiCond_Always); // Set height of title bar
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // Dark background color
    ImGui::Begin("Title Bar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    float windowWidth = ImGui::GetWindowWidth();
    float startBtnWidth = 80.0f;
    float validationBtnWidth = 100.0f;
    float toggleSceneBtnWidth = 120.0f;
    float totalBtnWidth = startBtnWidth + validationBtnWidth + toggleSceneBtnWidth + 20.0f; // + spacing
    float centerX = (windowWidth - totalBtnWidth) * 0.5f;
    float buttonHeight = 20.0f;
    float verticalOffset = (30.0f - buttonHeight) * 0.5f;

    // Define button colors
    ImVec4 titleColor = ImVec4(0.07f, 0.07f, 0.07f, 1.0f);  // Normal
    ImVec4 titleHoverColor = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);  // Hovered and active

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    ImGui::SetCursorPos(ImVec2(10.0f, 7.0f));
    ImGui::Text("x=%d y=%d", InputManager::GetMouseX(), InputManager::GetMouseY());
    ImGui::PopStyleColor();


    ImGui::PushStyleColor(ImGuiCol_Button, titleColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, titleHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, titleHoverColor);

    ImGui::SetCursorPos(ImVec2(centerX, verticalOffset));
    if (ImGui::Button("START", ImVec2(startBtnWidth, buttonHeight))) {
        // START button logic
    }

    ImGui::SameLine();
    if (ImGui::Button("Validation", ImVec2(validationBtnWidth, buttonHeight))) {
        // Validation button logic
    }

    ImGui::SameLine();
    // Store the position of the Toggle Scene button to properly position the dropdown
    ImVec2 toggleSceneBtnPos = ImGui::GetCursorScreenPos();
    if (ImGui::Button("Toggle Scene", ImVec2(toggleSceneBtnWidth, buttonHeight))) {
        // Toggle the dropdown visibility
        m_showSceneOptions = !m_showSceneOptions;
        m_showMeshOptions = false;
    }

    // Add Mesh button
    float addMeshBtnWidth = 80.0f;
    ImGui::SameLine(0, 10.0f); // Add some spacing
    ImVec2 addMeshBtnPos = ImGui::GetCursorScreenPos();
    if (ImGui::Button("Add Mesh", ImVec2(addMeshBtnWidth, buttonHeight))) {
        // Toggle the mesh options dropdown
        m_showMeshOptions = !m_showMeshOptions;
        m_showSceneOptions = false;
    }

    // Style for checkboxes and mini window to match other panels
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White checkmark
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f)); // Dark frame background
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f)); // Slightly lighter when hovered
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); // Slightly lighter when active

    // Check if we should show the scene options dropdown
    if (m_showSceneOptions) {
        // Set the position of the dropdown panel below the Toggle Scene button
        ImGui::SetNextWindowPos(ImVec2(toggleSceneBtnPos.x, toggleSceneBtnPos.y + buttonHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);

        // Set window background to match other panels
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        // Create a child window for the dropdown
        if (ImGui::Begin("Scene Options", &m_showSceneOptions,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings)) {

            // Grid Display checkbox
            ImGui::Checkbox("Grid Display", &m_showGrid);

            // Coordinate System checkbox
            ImGui::Checkbox("Coordinate System", &m_showCoordinateSystem);

            // Axis Orientation checkbox
            ImGui::Checkbox("Axis Orientation", &m_showAxisOrientation);

            // Scene Inspector checkbox
            ImGui::Checkbox("Scene Inspector", &m_showSceneInspector);

            // Performance Metrics checkbox
            ImGui::Checkbox("Performance Metrics", &m_showPerformanceMetrics);

            ImGui::End(); // End the child window
        }

        ImGui::PopStyleColor(2); // Pop the window background colors
        ImGui::PopStyleVar(2); // Pop the style variables
    }

    if (m_showMeshOptions) {
        ImGui::SetNextWindowPos(ImVec2(addMeshBtnPos.x, addMeshBtnPos.y + buttonHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

        // Style for full-width hover and white text
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));        // Hovered
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); // Same for hovered
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));  // Same for active
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));          // White text

        if (ImGui::Begin("Mesh Options", &m_showMeshOptions,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings)) {

            if (ImGui::Selectable("Plane", false, ImGuiSelectableFlags_SpanAllColumns)) {
                 loadMesh(MeshType::Plane);
            }
            if (ImGui::Selectable("Cube", false, ImGuiSelectableFlags_SpanAllColumns)) {
                 loadMesh(MeshType::Cube);
            }
            if (ImGui::Selectable("Sphere", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Sphere);
            }
            if (ImGui::Selectable("Cylinder", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Cylinder);
            }
            if (ImGui::Selectable("Disk", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Disk);
            }
            if (ImGui::Selectable("Trihedral Corner Reflector", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Trihedral);
            }
            if (ImGui::Selectable("Dihedral Reflector", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Dihedral);
            }
            if (ImGui::Selectable("Vertex Picking Mode", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::Picker);
            }
            if (ImGui::Selectable("AI Mesh Generation", false, ImGuiSelectableFlags_SpanAllColumns)) {
                loadMesh(MeshType::AI);
            }

            ImGui::End();
        }

        ImGui::PopStyleColor(4); // Text + header colors
        ImGui::PopStyleColor(2); // Window + popup
        ImGui::PopStyleVar(2);
    }

    ImGui::PopStyleColor(4); // Restore checkbox and frame colors
    ImGui::PopStyleColor(3); // Restore button colors
    ImGui::End();            // End title bar window
    ImGui::PopStyleColor();  // Restore title bar background color
}

void Application::drawCoordinateSystemImage()
{
    // Get the window size and the panel sizes
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float imageWidth = 125.0f;
    float imageHeight = 125.0f;

    // Set the next window position
    ImGui::SetNextWindowPos(ImVec2(1480, 50), ImGuiCond_Always);

    // Set the window size
    ImGui::SetNextWindowSize(ImVec2(imageWidth, imageHeight), ImGuiCond_Always);

    // Set window background to be fully transparent
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Use window flags to make everything transparent
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("Coordinate System", nullptr, window_flags);

    // Draw the image
    ImGui::Image((ImTextureID)(intptr_t)coordinateSystemTextureID, ImVec2(imageWidth, imageHeight));

    ImGui::End();

    // Don't forget to pop the style
    ImGui::PopStyleColor();
}

void Application::Run() {
    MainLoop();
}

void Application::MainLoop() {
    // FPS calculation variables
    static float lastTime = 0.0f;
    static int frameCount = 0;
    float fps = 0.0f;

    // CPU and RAM usage variables
    static float cachedCpuUsage = 0.0f;
    static float lastCpuQueryTime = 0.0f;
    static std::string cachedRamUsageStr = "";
    static std::string cachedGpuUsage = "";
    while (!glfwWindowShouldClose(window)) {

        // Time tracking
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS calculation (every second)
        frameCount++;
        if (currentFrame - lastTime >= 1.0f) { // Update FPS every 1 second
            fps = frameCount;  // Set FPS to the frame count over the last second
            frameCount = 0;    // Reset frame count
            lastTime = currentFrame; // Reset the time tracker
        }

        glfwPollEvents();

        if (renderer->m_InputManager.m_leftMouseButton.IsPressed) {
            renderer->drawPickingTexture();
        }

        // Set background color to dark gray for the screen
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f); // Darker gray background for the screen
        glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

        // Handles camera inputs
        if (!ImGui::GetIO().WantCaptureMouse) {
            renderer->camera.Inputs(window);
        }

        //----------------------------------------
        // -- Draw OpenGL 3D World --
        //----------------------------------------

        // Draw grid layout
        if(m_showGrid) renderer->drawGridLayout();

        // Draw coordinate system
        if(m_showCoordinateSystem) renderer->drawCoordinateSystem();

        // Draw scene collections
        renderer->drawSceneCollection();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Performance metrics
        // Throttle CPU usage update (once per second)
        if (currentFrame - lastCpuQueryTime >= 1.0f) {
            cachedRamUsageStr = GetRAMutilization();
            cachedCpuUsage = GetCPUutilization();
            cachedGpuUsage = GetGPUutilization();
            lastCpuQueryTime = currentFrame;
        }

        //----------------------------------------
        // -- Draw ImGui (UI World) --
        //---------------------------------------

        // Performance metrics
        if(m_showPerformanceMetrics) this->drawPerformanceMetrics(fps, cachedCpuUsage, cachedRamUsageStr, cachedGpuUsage);

        // Title bar
        InputManager::UpdateMousePosition(window);
        this->drawTitleBar();

        // First Right Panel: Scene Collection (40% of screen height)
        this->drawSceneCollection();

        // Second Right Panel: Object Editor (60% of screen height)
        this->drawObjectEditor();

        //Results Panel (Bottom Panel)
        this->drawResultsPanel();

        // Content Browser (Bottom Panel)
        this->drawContentBrowser();

        // Scene Inspector (Left Panel II)
        if(m_showSceneInspector) this->drawSceneInspector();

        // Coordinate system image
        if(m_showAxisOrientation) this->drawCoordinateSystemImage();

        // Object selection window
        this->renderObjectSelectionWindow();

        // Render mesh creator
        this->renderMeshCreator();

        // Render ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void Application::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.reset();

    glfwDestroyWindow(window);
    glfwTerminate();
}

