///////////////////////////////////////////////////////////////////////////////
// SceneManager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
// 
//  EDITORS:  Ethan Anderson - SNHU Student / Computer Science
// 
//  Assimp implementation adapted from learnopengl.com and Assimp documentation. Function references included in comments.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>
#include <functional>

// Assimp library for loading 3D models
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Nlohmann JSON Library for saving and loading scene data
#include <json.hpp>

// Filestream
#include <fstream>

using json = nlohmann::json;

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	// The following methods are for the students to 
	// customize for their own 3D scene
	void DefineObjectMaterials();
	void SetupSceneLights();
	void PrepareScene();
	void RenderScene();

	void LoadSceneTextures();

	// Seperate scene components into different methods with unique values
	void RenderKnobs();
	void RenderFloor();
	void RenderPictureFrame();
	void RenderVase();
	void RenderVaseBase();
	void RenderCandleHolders();
	void RenderCandles();
	void RenderCandleWicks();
	void RenderDoors();
	void RenderDrawers();
	void RenderNegativeSpace();
	void RenderBackdrop();
	void RenderCredenza();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

	// Mesh object structure to hold properties for each mesh
	struct MESH_OBJECT
	{
		std::string tag;
		glm::vec3 rotation;
		glm::vec3 position;
		glm::vec3 scale;
		std::string materialTag;
		std::string textureTag;
		glm::vec2 uvScale;
		glm::vec4 shaderColor;
		std::function<void()> drawFunction;
		bool isRotating = false;
	};

	// Add meshes to scene with various properties
	void AddMeshToScene(std::string tag, glm::vec3 position, glm::vec3 rotation, 
		glm::vec3 scale, std::string materialTag, 
		std::string textureTag, glm::vec2 uvScale,
		glm::vec4 shaderColor, std::function<void()> drawFunction);

	// Add basic shapes to the scene with unique properties
	void AddBox();
	void AddCone();
	void AddCylinder();
	void AddPlane();
	void AddPrism();
	void AddPyramid3();
	void AddPyramid4();
	void AddSphere();
	void AddTaperedCylinder();
	void AddTorus();

	// Render all the meshes in the scene
	void RenderMeshes();

	// Getters for the meshes
	int GetNumMeshes() { return m_meshes.size(); }

	// Get a specific mesh object by index
	MESH_OBJECT& GetMesh(int index) { return m_meshes[index]; }

	// Remove a mesh object from the scene by index
	void RemoveMesh(int index);

	// List of mesh objects to be rendered in the scene
	std::vector<MESH_OBJECT> m_meshes;

	// Load a 3D model from a file and process its meshes
	void LoadModel(std::string filename, std::string tag, 
		glm::vec3 position, glm::vec3 rotation,
		glm::vec3 scale, std::string materialTag,
		std::string textureTag, glm::vec2 uvScale,
		glm::vec4 shaderColor, bool isRotating);
	void ProcessNode(aiNode* node, const aiScene* scene, 
		std::string tag, glm::vec3 position, 
		glm::vec3 rotation, glm::vec3 scale,
		std::string materialTag,
		std::string textureTag, glm::vec2 uvScale,
		glm::vec4 shaderColor, bool isRotating);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, 
		std::string tag, glm::vec3 position, glm::vec3 rotation, 
		glm::vec3 scale, std::string materialTag,
		std::string textureTag, glm::vec2 uvScale,
		glm::vec4 shaderColor, bool isRotating);

	// Infinite rotation boolean
	bool isRotating = false;

	void SerializeSceneData(std::string filename);
	void DeserializeSceneData(std::string filename);

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);

};