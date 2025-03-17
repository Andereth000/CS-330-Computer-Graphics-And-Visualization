#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library
#include "imgui.h"		  // ImGui library
#include "backends/imgui_impl_glfw.h" // ImGui GLFW backend
#include "backends/imgui_impl_opengl3.h" // ImGui OpenGL3 backend

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
	// Macro for window title
	const char* const WINDOW_TITLE = "Credenza OpenGL"; 

	// Main GLFW window
	GLFWwindow* g_Window = nullptr;

	// scene manager object for managing the 3D scene prepare and render
	SceneManager* g_SceneManager = nullptr;
	// shader manager object for dynamic interaction with the shader code
	ShaderManager* g_ShaderManager = nullptr;
	// view manager object for managing the 3D view setup and projection to 2D
	ViewManager* g_ViewManager = nullptr;
}

// Function declarations - all functions that are called manually
// need to be pre-declared at the beginning of the source code.
bool InitializeGLFW();
bool InitializeGLEW();
bool InitializeImGui();
void ShutdownImGui();
void DrawImGui();

int curMeshIndex = -1;


/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
	// if GLFW fails initialization, then terminate the application
	if (InitializeGLFW() == false)
	{
		return(EXIT_FAILURE);
	}

	// try to create a new shader manager object
	g_ShaderManager = new ShaderManager();
	// try to create a new view manager object
	g_ViewManager = new ViewManager(
		g_ShaderManager);

	// try to create the main display window
	g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

	// if GLEW fails initialization, then terminate the application
	if (InitializeGLEW() == false)
	{
		return(EXIT_FAILURE);
	}

	// if ImGui fails initialization, then terminate the application
	if (InitializeImGui() == false)
	{
		return(EXIT_FAILURE);
	}

	// load the shader code from the external GLSL files
	g_ShaderManager->LoadShaders(
		"../../Utilities/shaders/vertexShader.glsl",
		"../../Utilities/shaders/fragmentShader.glsl");
	g_ShaderManager->use();

	// try to create a new scene manager object and prepare the 3D scene
	g_SceneManager = new SceneManager(g_ShaderManager);
	g_SceneManager->PrepareScene();

	// loop will keep running until the application is closed 
	// or until an error has occurred
	while (!glfwWindowShouldClose(g_Window))
	{
		// Enable z-depth
		glEnable(GL_DEPTH_TEST);

		// Clear the frame and z buffers
		// Sky color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// convert from 3D object space to 2D view
		g_ViewManager->PrepareSceneView();

		// refresh the 3D scene
		g_SceneManager->RenderScene();

		// Begin ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Draw ImGui elements
		DrawImGui();

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Flips the the back buffer with the front buffer every frame.
		glfwSwapBuffers(g_Window);

		// query the latest GLFW events
		glfwPollEvents();

	}

	// clear the allocated manager objects from memory
	if (NULL != g_SceneManager)
	{
		delete g_SceneManager;
		g_SceneManager = NULL;
	}
	if (NULL != g_ViewManager)
	{
		delete g_ViewManager;
		g_ViewManager = NULL;
	}
	if (NULL != g_ShaderManager)
	{
		delete g_ShaderManager;
		g_ShaderManager = NULL;
	}

	// Terminates the program successfully
	exit(EXIT_SUCCESS); 

	// Shutdown the ImGui library
	ShutdownImGui();
}

/***********************************************************
 *	InitializeGLFW()
 * 
 *  This function is used to initialize the GLFW library.   
 ***********************************************************/
bool InitializeGLFW()
{
	// GLFW: initialize and configure library
	// --------------------------------------
	glfwInit();

#ifdef __APPLE__
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// set the version of OpenGL and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	// GLFW: end -------------------------------

	return(true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
	// GLEW: initialize
	// -----------------------------------------
	GLenum GLEWInitResult = GLEW_OK;

	// try to initialize the GLEW library
	GLEWInitResult = glewInit();
	if (GLEW_OK != GLEWInitResult)
	{
		std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
		return false;
	}
	// GLEW: end -------------------------------

	// Displays a successful OpenGL initialization message
	std::cout << "INFO: OpenGL Successfully Initialized\n";
	std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

	return(true);
}

/***********************************************************
 *	InitializeImGui()
 *
 *  This function is used to initialize the imgui library.
 ***********************************************************/
bool InitializeImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(g_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	// Display a successful ImGui initialization message
	std::cout << "INFO: ImGui Successfully Initialized\n";

	return(true);
}

/***********************************************************
 *	DrawImGui()
 *
 *  This function draws the imgui ui.
 ***********************************************************/
void DrawImGui()
{
	ImGui::Begin("Scene Objects");

	if (ImGui::CollapsingHeader("Meshes"))
	{
		if (ImGui::Button("Add Box"))
		{
			g_SceneManager->AddBox();
		}
		if (ImGui::Button("Add Cone"))
		{
			g_SceneManager->AddCone();
		}
		if (ImGui::Button("Add Cylinder"))
		{
			g_SceneManager->AddCylinder();
		}
		if (ImGui::Button("Add Plane"))
		{
			g_SceneManager->AddPlane();
		}
		if (ImGui::Button("Add Prism"))
		{
			g_SceneManager->AddPrism();
		}
		if (ImGui::Button("Add Pyramid 3"))
		{
			g_SceneManager->AddPyramid3();
		}
		if (ImGui::Button("Add Pyramid 4"))
		{
			g_SceneManager->AddPyramid4();
		}
		if (ImGui::Button("Add Sphere"))
		{
			g_SceneManager->AddSphere();
		}
		if (ImGui::Button("Add Tapered Cylinder"))
		{
			g_SceneManager->AddTaperedCylinder();
		}
		if (ImGui::Button("Add Torus"))
		{
			g_SceneManager->AddTorus();
		}
	}

	if (g_SceneManager->GetNumMeshes() > 0)
	{
		ImGui::Separator();
		ImGui::Text("Edit Mesh Transform");

		// Select which mesh to edit
		ImGui::SliderInt("Selected Mesh", &curMeshIndex, 0, g_SceneManager->GetNumMeshes() - 1);

		if (curMeshIndex >= 0 && curMeshIndex < g_SceneManager->GetNumMeshes())
		{
			SceneManager::MESH_OBJECT& mesh = g_SceneManager->GetMesh(curMeshIndex);

			// Position Controls
			ImGui::DragFloat3("Position", &mesh.position.x, 0.1f, -10.0f, 10.0f);

			// Rotation Controls
			ImGui::DragFloat3("Rotation", &mesh.rotation.x, 1.0f, -180.0f, 180.0f);

			// Scale Controls
			ImGui::DragFloat3("Scale", &mesh.scale.x, 0.1f, 0.1f, 5.0f);

			// Material Controls
			ImGui::Text("Material");

			// To wire InputText() with std::string or any other custom string type,
			// see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
			// (Reference: https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp)
			static char materialTag[64];
			strncpy_s(materialTag, mesh.materialTag.c_str(), sizeof(materialTag) - 1);
			materialTag[sizeof(materialTag) - 1] = '\0';

			if (ImGui::InputText("Material##", materialTag, sizeof(materialTag)))
			{
				mesh.materialTag = std::string(materialTag);
			}

			// Texture Controls
			/*ImGui::Text("Texture");
			
			// To wire InputText() with std::string or any other custom string type,
			// see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
			// (Reference: https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp)
			static char textureTag[64];
			strncpy_s(textureTag, mesh.textureTag.c_str(), sizeof(textureTag) - 1);
			textureTag[sizeof(textureTag) - 1] = '\0';

			if (ImGui::InputText("Texture##", textureTag, sizeof(textureTag)))
			{
				mesh.textureTag = std::string(textureTag);
			}*/

			// UV Scale Controls
			ImGui::Text("UV Scale");
			ImGui::DragFloat2("UV Scale##", &mesh.uvScale.x, 0.1f, 0.1f, 10.0f);

			// Shader Color Controls
			ImGui::Text("Shader Color");
			ImGui::ColorEdit4("Shader Color##", &mesh.shaderColor.r);
			
		}

		// Delete Mesh
		if (ImGui::Button("Delete Mesh"))
		{
			g_SceneManager->RemoveMesh(curMeshIndex);
			curMeshIndex = std::max(0, curMeshIndex - 1);
		}
	}

	ImGui::End();

}

/***********************************************************
 *	ShutdownImGui()
 *
 *  This function is used to shutdown the imgui library.
 ***********************************************************/
void ShutdownImGui()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}