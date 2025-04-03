///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
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

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for loading textures
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"../../Utilities/textures/hardwood.jpg",
		"floor");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/gold-seamless-texture.jpg",
		"knobs");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/ornate_wood.png",
		"doors");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/rusticwood.jpg",
		"credenza");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stucco_wall.jpg",
		"backdrop");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/picture_frame.jpg",
		"picture frame");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/glass_texture1.png",
		"candle holders");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/glass_texture2.png",
		"vase");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless.jpg",
		"stainless");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{

	// Default material for all basic meshes in scene
	OBJECT_MATERIAL defaultMaterial;
	defaultMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	defaultMaterial.ambientStrength = 0.4f;
	defaultMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	defaultMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	defaultMaterial.shininess = 16.0;
	defaultMaterial.tag = "default";

	m_objectMaterials.push_back(defaultMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	metalMaterial.ambientStrength = 0.3f;
	metalMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	metalMaterial.shininess = 22.0;
	metalMaterial.tag = "metal";

	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3);
	woodMaterial.shininess = 22.0;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL frameMaterial;
	frameMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	frameMaterial.ambientStrength = 0.5f;
	frameMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	frameMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.01);
	frameMaterial.shininess = 80.0;
	frameMaterial.tag = "picture frame";

	m_objectMaterials.push_back(frameMaterial);

	OBJECT_MATERIAL woodNoShineMaterial;
	woodNoShineMaterial.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodNoShineMaterial.ambientStrength = 0.2f;
	woodNoShineMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	woodNoShineMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3);
	woodNoShineMaterial.shininess = 0.3;
	woodNoShineMaterial.tag = "woodNoShine";

	m_objectMaterials.push_back(woodNoShineMaterial);

	OBJECT_MATERIAL wallMaterial;
	wallMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	wallMaterial.ambientStrength = 0.2f;
	wallMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	wallMaterial.specularColor = glm::vec3(0.01f, 0.01f, 0.01f);
	wallMaterial.shininess = 3.0;
	wallMaterial.tag = "wall";

	m_objectMaterials.push_back(wallMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.01f);
	glassMaterial.shininess = 12.0;
	glassMaterial.tag = "glass";

	m_objectMaterials.push_back(glassMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Light 0
	m_pShaderManager->setVec3Value("lightSources[0].position", -50.0f, 30.0f, 0.0f);
	// Warm lighting
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.1f, 0.1f, 0.01f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 10.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.1f);

	// Light 1
	m_pShaderManager->setVec3Value("lightSources[1].position", 0.0f, 8.0f, 15.0f);
	// Warm lighting
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.1f, 0.1f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 5.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.1f);

	// Light 2
	m_pShaderManager->setVec3Value("lightSources[2].position", 50.0f, 30.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 10.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.1f);
	
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{

	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	// Preload all basic meshes
	m_basicMeshes->LoadBoxMesh(); // Cabinet/Drawers
	m_basicMeshes->LoadConeMesh(); // Candles
	m_basicMeshes->LoadCylinderMesh(); // Knobs/Candle Holders
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh(); // Vase
	m_basicMeshes->LoadTorusMesh();
}

void SceneManager::AddMeshToScene(std::string tag, glm::vec3 position, 
	glm::vec3 rotation, glm::vec3 scale, 
	std::string materialTag, std::string textureTag, glm::vec2 uvScale,
	glm::vec4 shaderColor, std::function<void()> drawFunction)
{

	MESH_OBJECT newMesh;
	newMesh.tag = tag;
	newMesh.position = position;
	newMesh.rotation = rotation;
	newMesh.scale = scale;
	newMesh.materialTag = materialTag;
	newMesh.textureTag = textureTag;
	newMesh.uvScale = uvScale;
	newMesh.shaderColor = shaderColor;
	newMesh.drawFunction = drawFunction;

	m_meshes.push_back(newMesh);

}

/***********************************************************
 *  AddBox()
 *
 *  This method is used for adding a box to the scene
 ***********************************************************/
void SceneManager::AddBox()
{
	AddMeshToScene("box", glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawBoxMesh(); });
}

/***********************************************************
 *  AddCone()
 *
 *  This method is used for adding a cone to the scene
 ***********************************************************/
void SceneManager::AddCone()
{
	AddMeshToScene("cone", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawConeMesh(); });
}

/***********************************************************
 *  AddCylinder()
 *
 *  This method is used for adding a cylinder to the scene
 ***********************************************************/
void SceneManager::AddCylinder()
{
	AddMeshToScene("cylinder", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawCylinderMesh(); });
}

/***********************************************************
 *  AddPlane()
 *
 *  This method is used for adding a plane to the scene
 ***********************************************************/
void SceneManager::AddPlane()
{
	AddMeshToScene("plane", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawPlaneMesh(); });
}

/***********************************************************
 *  AddPrism()
 *
 *  This method is used for adding a prism to the scene
 ***********************************************************/
void SceneManager::AddPrism()
{
	AddMeshToScene("prism", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawPrismMesh(); });
}

/***********************************************************
 *  AddPyramid3()
 *
 *  This method is used for adding a pyramid to the scene
 ***********************************************************/
void SceneManager::AddPyramid3()
{
	AddMeshToScene("pyramid3", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawPyramid3Mesh(); });
}

/***********************************************************
 *  AddPyramid4()
 *
 *  This method is used for adding a pyramid to the scene
 ***********************************************************/
void SceneManager::AddPyramid4()
{
	AddMeshToScene("pyramid4", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawPyramid4Mesh(); });
}

/***********************************************************
 *  AddSphere()
 *
 *  This method is used for adding a sphere to the scene
 ***********************************************************/
void SceneManager::AddSphere()
{
	AddMeshToScene("sphere", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawSphereMesh(); });
}

/***********************************************************
 *  AddTaperedCylinder()
 *
 *  This method is used for adding a tapered cylinder to the scene
 ***********************************************************/
void SceneManager::AddTaperedCylinder()
{
	AddMeshToScene("tapered cylinder", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawTaperedCylinderMesh(); });
}

/***********************************************************
 *  AddTorus()
 *
 *  This method is used for adding a torus to the scene
 ***********************************************************/
void SceneManager::AddTorus()
{
	AddMeshToScene("torus", glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f),
		"", "", { 1.0f, 1.0f }, { 1.0, 1.0, 1.0, 1.0 }, [this]() { m_basicMeshes->DrawTorusMesh(); });
}

/***********************************************************
 *  RenderMeshes()
 *
 *  This method is used for rendering all the basic meshes in the scene.
 ***********************************************************/
void SceneManager::RenderMeshes()
{
	// Loop through all the meshes in the scene and render them
	for (auto& mesh : m_meshes)
	{
		// Infinite rotation toggle
		if (isRotating)
		{
		mesh.rotation.y += 0.2f;
		if (mesh.rotation.y > 360.0f) mesh.rotation.y -= 360.0f;
		}

		SetTransformations(mesh.scale, mesh.rotation.x, mesh.rotation.y, mesh.rotation.z, mesh.position);
		SetShaderMaterial(mesh.materialTag);
		SetShaderTexture(mesh.textureTag);
		SetTextureUVScale(mesh.uvScale.x, mesh.uvScale.y);
		SetShaderColor(mesh.shaderColor.r, mesh.shaderColor.g, mesh.shaderColor.b, mesh.shaderColor.a);
		
		if (mesh.drawFunction)
		{
			mesh.drawFunction();
		}
	}
}

/***********************************************************
 *  RemoveMesh()
 *
 *  This method is used for removing a mesh from the scene
 ***********************************************************/
void SceneManager::RemoveMesh(int index)
{
	if (index >= 0 && index < m_meshes.size())
	{
		m_meshes.erase(m_meshes.begin() + index);
	}
}

/***********************************************************
 *  LoadModel()
 *
 *  This method is used for loading a 3D model from a file using Assimp.
 *  Adapted from https://learnopengl.com/Model-Loading/Model
 *  and https://assimp-docs.readthedocs.io/en/latest/
 ***********************************************************/
void SceneManager::LoadModel(std::string filename, std::string tag,
	glm::vec3 position, glm::vec3 rotation,
	glm::vec3 scale, std::string materialTag,
	std::string textureTag, glm::vec2 uvScale,
	glm::vec4 shaderColor, bool isRotating)
{
	Assimp::Importer importer;

	// aiProcess_GenSmoothNormals to fix teapot missing normals
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs| aiProcess_GenSmoothNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	ProcessNode(scene->mRootNode, scene, 
		tag, position, 
		rotation, scale, 
		materialTag, textureTag, 
		uvScale, shaderColor, 
		isRotating);
}

/***********************************************************
 *  ProcessNode()
 *
 *  This method is used for processing a node in the 3D model.
 *  Adapted from https://learnopengl.com/Model-Loading/Model
 *  and https://assimp-docs.readthedocs.io/en/latest/
 ***********************************************************/
void SceneManager::ProcessNode(aiNode* node, const aiScene* scene, 
	std::string tag, glm::vec3 position, 
	glm::vec3 rotation, glm::vec3 scale, 
	std::string materialTag, std::string textureTag, 
	glm::vec2 uvScale, glm::vec4 shaderColor, 
	bool isRotating)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		std::string meshTag = tag + std::to_string(i);
		ProcessMesh(mesh, scene, 
			tag + std::to_string(i), position, 
			rotation, scale, 
			materialTag, textureTag,
			uvScale, shaderColor,
			isRotating);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, 
			tag, position, 
			rotation, scale,
			materialTag, textureTag,
			uvScale, shaderColor,
			isRotating);
	}
}

/***********************************************************
 *  ProcessMesh()
 *
 *  This method is used for processing meshes in the 3D model (without textures).
 *  Adapted from https://learnopengl.com/Model-Loading/Model
 *  and https://learnopengl.com/Model-Loading/Mesh
 *  and https://assimp-docs.readthedocs.io/en/latest/
 ***********************************************************/
void SceneManager::ProcessMesh(aiMesh* mesh, const aiScene* scene, 
	std::string tag, glm::vec3 position, 
	glm::vec3 rotation, glm::vec3 scale,
	std::string materialTag, std::string textureTag, 
	glm::vec2 uvScale, glm::vec4 shaderColor, bool isRotating)
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.push_back(mesh->mVertices[i].x);
		vertices.push_back(mesh->mVertices[i].y);
		vertices.push_back(mesh->mVertices[i].z);

		if (mesh->HasNormals())
		{
			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);
		}
		else
		{
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
		}
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Initialize buffers
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Bind vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Position (stride of 6 without texture coordinates)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal (stride of 6 without texture coordinates)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Create a draw function for the mesh
	std::function<void()> drawFunction = [VAO, indices]() {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		};

	// Add the mesh to the scene
	AddMeshToScene(tag, position, 
		rotation, scale, 
		materialTag, textureTag, 
		uvScale, shaderColor, 
		drawFunction);
	m_meshes.back().isRotating = isRotating;
}

/***********************************************************
 *  SerializeSceneData()
 *
 *  This method is used for saving scene data to a file
 *	(Reference: https://json.nlohmann.me/features/arbitrary_types/)
 ***********************************************************/
void SceneManager::SerializeSceneData(std::string filename)
{
	json jScene;

	for (auto& mesh : m_meshes)
	{
		json jMesh;
		jMesh["tag"] = mesh.tag;
		jMesh["position"] = { mesh.position.x, mesh.position.y, mesh.position.z };
		jMesh["rotation"] = { mesh.rotation.x, mesh.rotation.y, mesh.rotation.z };
		jMesh["scale"] = { mesh.scale.x, mesh.scale.y, mesh.scale.z };
		jMesh["materialTag"] = mesh.materialTag;
		jMesh["textureTag"] = mesh.textureTag;
		jMesh["uvScale"] = { mesh.uvScale.x, mesh.uvScale.y };
		jMesh["shaderColor"] = { mesh.shaderColor.r, mesh.shaderColor.g, mesh.shaderColor.b, mesh.shaderColor.a };
		jMesh["isRotating"] = mesh.isRotating;

		jScene.push_back(jMesh);

	}
	std::ofstream file(filename);

	// Better JSON formatting
	file << jScene.dump(4);

	file.close();
}

 /***********************************************************
  *  DeserializeSceneData()
  *
  *  This method is used for loading scene data from a file
  *	 (Reference: https://json.nlohmann.me/features/arbitrary_types/)
  ***********************************************************/
void SceneManager::DeserializeSceneData(std::string filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Could not open file: " << filename << std::endl;
		return;
	}

	json jScene;
	file >> jScene;
	file.close();

	m_meshes.clear();

	for (auto& jMesh : jScene)
	{
		// Retrieve model data
		std::string tag = jMesh["tag"];
		glm::vec3 position(jMesh["position"][0], jMesh["position"][1], jMesh["position"][2]);
		glm::vec3 rotation(jMesh["rotation"][0], jMesh["rotation"][1], jMesh["rotation"][2]);
		glm::vec3 scale(jMesh["scale"][0], jMesh["scale"][1], jMesh["scale"][2]);
		std::string materialTag = jMesh["materialTag"];
		std::string textureTag = jMesh["textureTag"];
		glm::vec2 uvScale(jMesh["uvScale"][0], jMesh["uvScale"][1]);
		glm::vec4 shaderColor(jMesh["shaderColor"][0], jMesh["shaderColor"][1], jMesh["shaderColor"][2], jMesh["shaderColor"][3]);
		bool isRotating = jMesh["isRotating"];

		// Handle re-drawing models
		if (tag.find("Stanford Bunny") != std::string::npos)
		{
			LoadModel("../../Models/bunny.obj", tag, 
				position, rotation, 
				scale, materialTag, 
				textureTag, uvScale,
				shaderColor, isRotating);
			continue;
		}
		else if (tag.find("Lucy") != std::string::npos)
		{
			LoadModel("../../Models/lucy.obj", tag,
				position, rotation,
				scale, materialTag,
				textureTag, uvScale,
				shaderColor, isRotating);
			continue;
		}
		else if (tag.find("Suzanne") != std::string::npos)
		{
			LoadModel("../../Models/suzanne.obj", tag,
				position, rotation,
				scale, materialTag,
				textureTag, uvScale,
				shaderColor, isRotating);
			continue;
		}  
		else if (tag.find("Teapot") != std::string::npos)
		{
			LoadModel("../../Models/teapot.obj", tag,
				position, rotation,
				scale, materialTag,
				textureTag, uvScale,
				shaderColor, isRotating);
			continue;
		}
		
		// Retrive mesh data
		MESH_OBJECT mesh;
		mesh.tag = tag;
		mesh.position = position;
		mesh.rotation = rotation;
		mesh.scale = scale;
		mesh.materialTag = materialTag;
		mesh.textureTag = textureTag;
		mesh.uvScale = uvScale;
		mesh.shaderColor = shaderColor;
		mesh.isRotating = isRotating;

		// Handle re-drawing basic meshes
		if (tag.find("box") != std::string::npos)
		{
			AddMeshToScene(tag, position, 
				rotation, scale, 
				materialTag, textureTag, 
				uvScale, shaderColor, 
				[this]() { m_basicMeshes->DrawBoxMesh(); });
			continue;
		}
		else if (tag.find("cone") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawConeMesh(); });
			continue;
		}
		else if (tag.find("tapered cylinder") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawTaperedCylinderMesh(); });
			continue;
		}
		else if (tag.find("cylinder") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawCylinderMesh(); });
			continue;
		}
		else if (tag.find("plane") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawPlaneMesh(); });
			continue;
		}
		else if (tag.find("prism") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawPrismMesh(); });
			continue;
		}
		else if (tag.find("pyramid3") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawPyramid3Mesh(); });
			continue;
		}
		else if (tag.find("pyramid4") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawPyramid4Mesh(); });
			continue;
		}
		else if (tag.find("sphere") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawSphereMesh(); });
			continue;
		}
		else if (tag.find("torus") != std::string::npos)
		{
			AddMeshToScene(tag, position,
				rotation, scale,
				materialTag, textureTag,
				uvScale, shaderColor,
				[this]() { m_basicMeshes->DrawTorusMesh(); });
			continue;
		}

		m_meshes.push_back(mesh);

	}
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{

	RenderBackdrop();
	RenderFloor();
	RenderPictureFrame();
	RenderVase();
	RenderVaseBase();
	RenderCandleHolders();
	RenderCandles();
	RenderCandleWicks();
	RenderCredenza();
	RenderNegativeSpace();
	RenderDrawers();
	RenderDoors();
	RenderKnobs();

	// Allow rendering of all basic meshes
	RenderMeshes();
}


/***********************************************************
 *  RenderBackrop()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderBackdrop()
{
	SetShaderMaterial("wall");

	SetTextureUVScale(6, 5);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 7.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 5.0f, 3.3f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gray
	SetShaderColor(0.6, 0.6, 0.6, 1);
	SetShaderTexture("backdrop");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
}


/***********************************************************
 *  RenderFloor()
 *
 *  This method is called to render the shapes for the scene
 *  floor object.
 ***********************************************************/
void SceneManager::RenderFloor()
{
	SetShaderMaterial("wood");

	// Set texture scale for floor
	SetTextureUVScale(5, 10);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(10.0f, 1.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm:: vec3(0.0f, 0.0f, 10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Brown
	//SetShaderColor(0.803, 0.521, 0.247, 1);
	SetShaderTexture("floor");
	
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderPictureFrame()
 *
 *  This method is called to render the shapes for the scene
 *  picture frame object.
 ***********************************************************/
void SceneManager::RenderPictureFrame()
{
	SetShaderMaterial("picture frame");

	// Set texture scale for picture frame
	SetTextureUVScale(1, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(3.0f, 1.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 7.5f, 3.35f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Brown
	//SetShaderColor(0.803, 0.521, 0.247, 1);
	SetShaderTexture("picture frame");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderVase()
 *
 *  This method is called to render the shapes for the scene
 *  vase object.
 ***********************************************************/
void SceneManager::RenderVase()
{
	SetShaderMaterial("glass");

	// Set texture scale
	SetTextureUVScale(2, 2);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.6f, 1.0f, 0.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 5.301f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(1, 1, 1, 0.4);
	SetShaderTexture("vase");

	// draw the mesh with transformation values
	m_basicMeshes->DrawTaperedCylinderMesh(true, false, true);
	/****************************************************************/
}

/***********************************************************
 *  RenderVaseBase()
 *
 *  This method is called to render the shapes for the scene
 *  vase base objects.
 ***********************************************************/
void SceneManager::RenderVaseBase()
{
	SetShaderMaterial("metal");

	// Set texture scale
	SetTextureUVScale(1, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 4.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(1, 1, 1, 0.4);
	SetShaderTexture("stainless");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.4f, 0.1f, 0.4f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 4.1f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(1, 1, 1, 0.4);
	SetShaderTexture("stainless");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.3f, 0.1f, 0.3f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 4.2f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(1, 1, 1, 0.4);
	SetShaderTexture("stainless");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/
}

/***********************************************************
 *  RenderCandleHolders()
 *
 *  This method is called to render the shapes for the scene
 *  candle holder objects.
 ***********************************************************/
void SceneManager::RenderCandleHolders()
{
	// Set texture scale
	SetTextureUVScale(2.5, 2);
	

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.4f, 0.8f, 0.4f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 4.01f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderMaterial("glass");
	SetShaderTexture("candle holders");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(false, true, true);
	/****************************************************************/

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.4f, 0.8f, 0.4f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.5f, 4.01f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(1, 1, 1, 0.5);
	SetShaderMaterial("glass");
	SetShaderTexture("candle holders");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(false, true, true);
	/****************************************************************/
}

/***********************************************************
 *  RenderCandles()
 *
 *  This method is called to render the shapes for the scene
 *  candle objects.
 ***********************************************************/
void SceneManager::RenderCandles()
{
	// Set texture scale
	SetTextureUVScale(2.5, 2);


	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.2f, 0.6f, 0.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 4.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.952, 0.890, 0.760, 1.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.2f, 0.6f, 0.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.5f, 4.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.952, 0.890, 0.760, 1.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, false, true);
	/****************************************************************/
}

/***********************************************************
 *  RenderCandles()
 *
 *  This method is called to render the shapes for the scene
 *  candle wick objects.
 ***********************************************************/
void SceneManager::RenderCandleWicks()
{
	// Set texture scale
	SetTextureUVScale(2.5, 2);


	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.005f, 0.1f, 0.005f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 4.55f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderMaterial("metal");
	SetShaderColor(0, 0, 0, 1.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.005f, 0.1f, 0.005f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.5f, 4.55f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderMaterial("metal");
	SetShaderColor(0, 0, 0, 1.0);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(true, true, true);
	/****************************************************************/
}

/***********************************************************
 *  RenderCredenza()
 *
 *  This method is called to render the shapes for the 
 *  credenza object.
 ***********************************************************/
void SceneManager::RenderCredenza()
{
	SetShaderMaterial("woodNoShine");

	SetTextureUVScale(1, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Center of credenza

	/*****************************Middle Piece*************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(4.0f, 4.0f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 2.0f, 5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("credenza");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	// Left side of credenza

	/***********************Left Piece*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 4.0f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.5f, 2.0f, 4.8f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("credenza");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	// Right side of credenza

	/***********************Right Piece*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.0f, 4.0f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.5f, 2.0f, 4.8f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("credenza");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderNegativeSpace()
 *
 *  This method is called to render the shapes for the 
 *  negative space object.
 ***********************************************************/
void SceneManager::RenderNegativeSpace()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************************Top Drawer Negative Space************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(3.6f, 0.6f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 3.5f, 5.05f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/*******************************Large Doors Negative Space***********************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(3.6f, 2.8f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 1.65f, 5.05f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Left Piece Top Drawer Negative Space*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.7f, 0.6f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.5f, 3.5f, 4.85f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Left Piece Bottom Door Negative Space*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.7f, 2.9f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.5f, 1.6f, 4.85f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Right Piece Top Drawer Negative Space*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.7f, 0.6f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.5f, 3.5f, 4.85f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Right Piece Bottom Door Negative Space*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.7f, 2.9f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.5f, 1.6f, 4.85f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Black
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderDrawers()
 *
 *  This method is called to render the shapes for the drawer 
 *  objects.
 ***********************************************************/
void SceneManager::RenderDrawers()
{
	SetShaderMaterial("wood");

	SetTextureUVScale(4, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/****************************Top Drawer**************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(3.52f, 0.52f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 3.51f, 5.06f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Left Piece Top Drawer*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.62f, 0.55f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.47f, 3.5f, 4.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Right Piece Top Drawer*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.62f, 0.55f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.47f, 3.5f, 4.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderDoors()
 *
 *  This method is called to render the shapes for the door 
 *  objects.
 ***********************************************************/
void SceneManager::RenderDoors()
{
	SetShaderMaterial("wood");

	SetTextureUVScale(1, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/********************************Large Door Left**********************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.75f, 2.74f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.89f, 1.66f, 5.06f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	//SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/



	/*******************************Large Door Right***********************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.75f, 2.74f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.89f, 1.66f, 5.06f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	//SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");
	
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Left Piece Bottom Door*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.65f, 2.8f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.48f, 1.62f, 4.86f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	//SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	/***********************Right Piece Bottom Door*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.65f, 2.8f, 2.6f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.48f, 1.62f, 4.86f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Offwhite
	//SetShaderColor(0.960, 0.960, 0.862, 1);
	SetShaderTexture("doors");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}

/***********************************************************
 *  RenderKnobs()
 *
 *  This method is called to render the shapes for the knob 
 *  objects.
 ***********************************************************/
void SceneManager::RenderKnobs()
{
	SetShaderMaterial("metal");

	SetTextureUVScale(1, 1);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/***********************Right Piece Bottom Door Knob*******************************************/
	// Set texture scale for all knobs
	SetTextureUVScale(1, 1);

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.25f, 1.62f, 6.15f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/***********************Right Piece Top Drawer Knob*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.47f, 3.5f, 6.15f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/***********************Left Piece Bottom Door Knob*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.25f, 1.62f, 6.15f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/***********************Left Piece Top Drawer Knob*******************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.47f, 3.5f, 6.15f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/****************************Large Door Right Knob**************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.2f, 1.65f, 6.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/****************************Large Door Left Knob**************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-0.2f, 1.65f, 6.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/****************************Top Drawer Knob**************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.08f, 0.08f, 0.08f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 3.5f, 6.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Gold
	//SetShaderColor(0.854, 0.647, 0.125, 1);
	SetShaderTexture("knobs");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/
}
