///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
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

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

void SceneManager::DefineObjectMaterials()
{
	m_objectMaterials.clear();

	// Snow / ground: bluish, low ambient 
	m_objectMaterials.push_back({
		/*ambientStrength*/ 0.18f,
		/*ambientColor*/    glm::vec3(0.70f, 0.78f, 0.92f),  // blue-white
		/*diffuseColor*/    glm::vec3(0.80f, 0.88f, 0.98f),
		/*specularColor*/   glm::vec3(0.15f),
		/*shininess*/       8.0f,
		/*tag*/             "snow"
		});

	// House neutral/cool
	m_objectMaterials.push_back({
		0.15f,
		glm::vec3(0.62f, 0.62f, 0.70f),
		glm::vec3(0.70f, 0.70f, 0.78f),
		glm::vec3(0.18f),
		12.0f,
		"house"
		});

}

void SceneManager::SetupSceneLights()
{
	// Enable custom lighting in the shader
	m_pShaderManager->setBoolValue("bUseLighting", true);

	for (int i = 0; i < 4; ++i) {
		std::string base = "lightSources[" + std::to_string(i) + "].";
		m_pShaderManager->setVec3Value(base + "position", glm::vec3(0.0f));
		m_pShaderManager->setVec3Value(base + "ambientColor", glm::vec3(0.0f));
		m_pShaderManager->setVec3Value(base + "diffuseColor", glm::vec3(0.0f));
		m_pShaderManager->setVec3Value(base + "specularColor", glm::vec3(0.0f));
		m_pShaderManager->setFloatValue(base + "focalStrength", 1.0f);
		m_pShaderManager->setFloatValue(base + "specularIntensity", 0.0f);
	}

		//— cool moonlight L0
	{
		const char* B = "lightSources[0].";
		m_pShaderManager->setVec3Value(std::string(B) + "position", glm::vec3(6.0f, 7.0f, 3.0f));
		m_pShaderManager->setVec3Value(std::string(B) + "ambientColor", glm::vec3(0.02f, 0.03f, 0.05f)); // tiny ambient
		m_pShaderManager->setVec3Value(std::string(B) + "diffuseColor", glm::vec3(0.65f, 0.75f, 1.00f)); // strong cool
		m_pShaderManager->setVec3Value(std::string(B) + "specularColor", glm::vec3(0.85f, 0.90f, 1.00f));
		m_pShaderManager->setFloatValue(std::string(B) + "focalStrength", 32.0f);
		m_pShaderManager->setFloatValue(std::string(B) + "specularIntensity", 0.60f);
	}

		//— lavender fill from left-back L1
	{
		const char* B = "lightSources[1].";
		m_pShaderManager->setVec3Value(std::string(B) + "position", glm::vec3(-6.0f, 4.0f, -4.0f));
		m_pShaderManager->setVec3Value(std::string(B) + "ambientColor", glm::vec3(0.00f));                 // no ambient
		m_pShaderManager->setVec3Value(std::string(B) + "diffuseColor", glm::vec3(0.55f, 0.45f, 0.70f));   // lavender
		m_pShaderManager->setVec3Value(std::string(B) + "specularColor", glm::vec3(0.20f, 0.16f, 0.28f));
		m_pShaderManager->setFloatValue(std::string(B) + "focalStrength", 16.0f);
		m_pShaderManager->setFloatValue(std::string(B) + "specularIntensity", 0.20f);
	}

}

GLuint SceneManager::LoadTexture2D(const char* path, bool flipY)
{
	int w, h, n;
	stbi_set_flip_vertically_on_load(flipY);
	unsigned char* data = stbi_load(path, &w, &h, &n, 0);
	if (!data) { std::cerr << "Failed to load texture: " << path << std::endl; return 0; }

	GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
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
	DefineObjectMaterials();
	SetupSceneLights();

	// House pieces
	m_basicMeshes->LoadBoxMesh();        // body, porch, frames
	m_basicMeshes->LoadCylinderMesh();   // chimney cap
	m_basicMeshes->LoadPrismMesh();      // roof
	m_basicMeshes->LoadPlaneMesh();      // ground/backdrop 
	m_basicMeshes->LoadConeMesh();         // foliage

	// --- Load house textures ---
	m_texBrick = LoadTexture2D("assets/textures/Brick.jpg");   // CC0 brick jpg
	// roof shingles:
	m_texRoof = LoadTexture2D("assets/textures/Roof.jpg");    // CC0 roof jpg

	// Tell shader which texture unit the sampler uses (unit 0)
	if (m_pShaderManager)
		m_pShaderManager->setIntValue("objectTexture", 0);  

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// ---------- working vars ----------
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	auto ApplyMat = [&](const char* tag)
		{
			for (const auto& M : m_objectMaterials)
			{
				if (M.tag == tag)
				{
					m_pShaderManager->setFloatValue("material.ambientStrength", M.ambientStrength);
					m_pShaderManager->setVec3Value("material.ambientColor", M.ambientColor);
					m_pShaderManager->setVec3Value("material.diffuseColor", M.diffuseColor);
					m_pShaderManager->setVec3Value("material.specularColor", M.specularColor);
					m_pShaderManager->setFloatValue("material.shininess", M.shininess);
					break;
				}
			}
		};


	// Enable/disable a 2D texture for the next draw
	auto UseTexture2D = [&](GLuint texID)
		{
			if (texID != 0)
			{
				m_pShaderManager->setIntValue("bUseTexture", true);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texID);
			}
			else
			{
				m_pShaderManager->setIntValue("bUseTexture", false);
			}
		};

	// Optionally set UV tiling 
	auto SetUV = [&](float u, float v)
		{
			if (m_pShaderManager) m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
		};

	// ---------- palette ----------
	const glm::vec4 STONE = glm::vec4(0.78f, 0.78f, 0.84f, 1.0f); // body (light)
	const glm::vec4 TRIM = glm::vec4(0.64f, 0.64f, 0.72f, 1.0f); // trim (brighter)
	const glm::vec4 ROOF = glm::vec4(0.64f, 0.60f, 0.70f, 1.0f); // roof lavender
	const glm::vec4 DOOR = glm::vec4(0.12f, 0.10f, 0.14f, 1.0f); // darker
	const glm::vec4 GLASS = glm::vec4(0.60f, 0.85f, 0.92f, 1.0f); // darker cyan
	const glm::vec3 H = glm::vec3(0.0f, -0.55f, 2.8f); //house anchor

	// debugging contrast
	 /*const glm::vec4 DOOR  = glm::vec4(1,0,0,1);
	 const glm::vec4 GLASS = glm::vec4(0,1,0,1);*/


	 // --- global scene nudges---
	const float YF = -4.0f;                // small yaw for the whole scene

	// ---- common anchors & nudges ----

	const float BODY_Z = +0.10f;
	const float FRONT_Z = 1.26f;
	const float EPS_Z = 0.04f;  // tiny forward nudge to avoid z-fighting
	const float FASCIA_Z = FRONT_Z;  // fascia/trim sits at the front
	const float ROOF_Z = 0.08f;    // how far forward the roof planes sit
	const float ROOF_Y = 2.10f;
	const float FASCIA_Y = 1.60f;    // fascia height
	const float PORCH_Y = -1.35f;
	const float STEP_Y = -1.52f;   // step height
	const float CHIMNEY_X = 0.90f;
	const float CHIMNEY_Z = -0.60f;
	const float CHIMNEY_BASE_Y = ROOF_Y + 1.45f;
	const float CHIMNEY_CAP_Y = CHIMNEY_BASE_Y + 0.95f;

	// helper to position relative to H
	auto P = [&](float x, float y, float z) { return H + glm::vec3(x, y, z); };


	// ---------- helper: capture *this* so member calls compile ----------
	auto DrawBox = [this](glm::vec3 s, glm::vec3 rDegXYZ, glm::vec3 p, glm::vec4 color)
		{
			this->SetTransformations(s, rDegXYZ.x, rDegXYZ.y, rDegXYZ.z, p);
			this->SetShaderColor(color.r, color.g, color.b, color.a);
			this->m_basicMeshes->DrawBoxMesh();
		};

	auto DrawTree = [&](glm::vec3 basePos, float trunkH, float trunkR, float crownH, float crownR)
		{
			// tree trunk
			SetTransformations(glm::vec3(trunkR, trunkH, trunkR), 0, 0, 0, basePos + glm::vec3(0.0f, trunkH * 0.5f, 0.0f));
			m_pShaderManager->setIntValue("bUseTexture", false);
			// cool brown 
			SetShaderColor(0.35f, 0.30f, 0.28f, 1.0f);
			m_basicMeshes->DrawCylinderMesh();

			// foliage — slightly above trunk top
			glm::vec3 crownPos = basePos + glm::vec3(0.0f, trunkH + crownH * 0.5f, 0.0f);
			SetTransformations(glm::vec3(crownR, crownH, crownR), 0, 0, 0, crownPos);
			// evergreen tone
			SetShaderColor(0.55f, 0.70f, 0.68f, 1.0f);
			m_basicMeshes->DrawConeMesh();

			// snowy cap 
			SetTransformations(glm::vec3(crownR * 0.55f, 0.08f, crownR * 0.55f), 0, 0, 0,
				basePos + glm::vec3(0.0f, trunkH + crownH - 0.02f, 0.0f));
			SetShaderColor(0.90f, 0.95f, 1.0f, 1.0f);
			m_basicMeshes->DrawCylinderMesh();
		};

	auto DrawFenceLine = [&](glm::vec3 start, glm::vec3 dir, int posts, float spacing)
		{
			// two horizontal rails 
			glm::vec3 mid = start + dir * (spacing * (posts - 1) * 0.5f);
			// lower rail
			SetTransformations(glm::vec3(spacing * posts, 0.05f, 0.12f), 0, 0, 0, mid + glm::vec3(0, -0.30f, 0));
			m_pShaderManager->setIntValue("bUseTexture", false);
			SetShaderColor(0.55f, 0.53f, 0.56f, 1.0f);  // desaturated wood/stone
			m_basicMeshes->DrawBoxMesh();
			// upper rail
			SetTransformations(glm::vec3(spacing * posts, 0.05f, 0.12f), 0, 0, 0, mid + glm::vec3(0, 0.05f, 0));
			SetShaderColor(0.58f, 0.56f, 0.60f, 1.0f);
			m_basicMeshes->DrawBoxMesh();

			// posts
			for (int i = 0; i < posts; ++i)
			{
				glm::vec3 p = start + dir * (spacing * i);
				SetTransformations(glm::vec3(0.10f, 0.60f, 0.10f), 0, 0, 0, p + glm::vec3(0, 0.15f, 0));
				SetShaderColor(0.50f, 0.48f, 0.52f, 1.0f);
				m_basicMeshes->DrawBoxMesh();
			}
		};

	//// --- Mountain  ---
	//auto DrawMountain = [this](glm::vec3 pos,
	//	float baseRadius,
	//	float height,
	//	glm::vec4 bodyColor,
	//	glm::vec4 capColor)
	//	{
	//		// body (cone)
	//		SetTransformations(glm::vec3(baseRadius, height, baseRadius), 0, 0, 0, pos);
	//		m_pShaderManager->setIntValue("bUseTexture", false);
	//		SetShaderColor(bodyColor.r, bodyColor.g, bodyColor.b, bodyColor.a);
	//		m_basicMeshes->DrawConeMesh();

	//		// snow cap 
	//		glm::vec3 capPos = pos + glm::vec3(0.0f, height * 0.75f, 0.0f);
	//		SetTransformations(glm::vec3(baseRadius * 0.37f, 0.12f, baseRadius * 0.37f), 0, 0, 0, capPos);
	//		SetShaderColor(capColor.r, capColor.g, capColor.b, capColor.a);
	//		m_basicMeshes->DrawCylinderMesh();
	//	};



	// ---------------- BACKDROP / FLOOR ----------------

	// Background wall
	SetTransformations(
		/*scale*/     glm::vec3(60.0f, 1.0f, 40.0f),
		/*rot XYZ*/   90.0f, 0.0f, 0.0f,
		/*position*/  glm::vec3(0.0f, 14.0f, -35.0f));
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderColor(0.28f, 0.22f, 0.42f, 1.0f);   // dusk purple
	m_basicMeshes->DrawPlaneMesh();

	// Ground (flat, bluish snow)
	SetTransformations(
		/*scale*/     glm::vec3(60.0f, 1.0f, 60.0f),
		/*rot XYZ*/   -90.0f, 0.0f, 0.0f,
		/*position*/  glm::vec3(0.0f, -2.0f, 0.0f));
	SetShaderMaterial("snow");
	m_pShaderManager->setIntValue("bUseTexture", false);
	SetShaderColor(0.80f, 0.88f, 0.98f, 1.0f);   // blue-white snow
	m_basicMeshes->DrawPlaneMesh();

	// ---------------- HOUSE ----------------

	// --- HOUSE BODY (Brick, tiled) ---
	SetTransformations(glm::vec3(3.90f, 3.80f, 2.70f), /*X*/0, /*Y*/-4.0f, /*Z*/0,
		/*pos*/   H + glm::vec3(0.0f, 0.0f, 0.10f));
	SetShaderColor(1, 1, 1, 1);
	SetShaderMaterial("house");
	UseTexture2D(m_texBrick);
	SetUV(3.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();
	UseTexture2D(0);


	// --- LEFT BUMP-OUT (Brick, same tile) ---
	SetTransformations(glm::vec3(1.50f, 2.40f, 2.20f), 0, -4.0f, 0,
		H + glm::vec3(-1.60f, -0.10f, 0.20f));
	SetShaderColor(1, 1, 1, 1);
	SetShaderMaterial("house");
	UseTexture2D(m_texBrick);
	SetUV(3.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();
	UseTexture2D(0);


	// Right front corner trim 
	DrawBox(glm::vec3(0.06f, 3.80f, 0.06f),
		glm::vec3(0, -4.0f, 0),
		P(+1.82f, 0.0f, FRONT_Z),   // on the front face
		TRIM);

	// Door
	DrawBox(glm::vec3(0.86f, 1.52f, 0.08f),
		glm::vec3(0, -4.0f, 0),
		P(0.00f, -0.55f, FRONT_Z + EPS_Z),
		DOOR);

	// Door frame 
	DrawBox(glm::vec3(0.92f, 1.58f, 0.02f),
		glm::vec3(0, -4.0f, 0),
		P(0.00f, -0.55f, FRONT_Z + EPS_Z + 0.02f),
		TRIM);

	// Left window (bump-out)
	DrawBox(glm::vec3(0.62f, 0.62f, 0.05f),
		glm::vec3(0, -4.0f, 0),
		P(-1.60f, 0.32f, FRONT_Z + EPS_Z),
		GLASS);
	DrawBox(glm::vec3(0.68f, 0.68f, 0.01f),
		glm::vec3(0, -4.0f, 0),
		P(-1.60f, 0.32f, FRONT_Z + EPS_Z + 0.02f),
		TRIM);

	// Right window (body)
	DrawBox(glm::vec3(0.70f, 0.92f, 0.05f),
		glm::vec3(0, -4.0f, 0),
		P(+1.45f, 0.28f, FRONT_Z + EPS_Z),
		GLASS);
	DrawBox(glm::vec3(0.76f, 0.98f, 0.01f),
		glm::vec3(0, -4.0f, 0),
		P(+1.45f, 0.28f, FRONT_Z + EPS_Z + 0.02f),
		TRIM);

	// ------------ ROOF ------------

// --- ROOF LEFT SLOPE ---
	SetTransformations(glm::vec3(1.95f, 0.25f, 3.05f), 0, -4.0f, +30.0f,
		H + glm::vec3(-0.78f, 3.00f, 0.06f));
	SetShaderColor(1, 1, 1, 1);
	SetShaderMaterial("house");
	UseTexture2D(m_texRoof ? m_texRoof : m_texBrick);  //fallback to brick if roof won't render
	SetUV(3.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();
	UseTexture2D(0);


	// --- ROOF RIGHT SLOPE ---
	SetTransformations(glm::vec3(1.95f, 0.25f, 3.05f), 0, -4.0f, -30.0f,
		H + glm::vec3(+0.78f, 3.00f, 0.06f));
	SetShaderColor(1, 1, 1, 1);
	SetShaderMaterial("house");
	UseTexture2D(m_texRoof ? m_texRoof : m_texBrick);;
	SetUV(3.0f, 2.0f);
	m_basicMeshes->DrawBoxMesh();
	UseTexture2D(0);



	/// stack
	DrawBox(glm::vec3(0.45f, 1.10f, 0.45f),
		glm::vec3(0.0f, -16.0f, 0.0f),
		P(CHIMNEY_X, CHIMNEY_BASE_Y, CHIMNEY_Z),
		TRIM);

	// cap
	SetTransformations(glm::vec3(0.60f, 0.12f, 0.60f), 0, 0, 0,
		P(CHIMNEY_X, CHIMNEY_CAP_Y, CHIMNEY_Z));
	SetShaderColor(TRIM.r, TRIM.g, TRIM.b, TRIM.a);
	SetShaderMaterial("house");
	m_pShaderManager->setIntValue("bUseTexture", false);
	m_pShaderManager->setVec4Value("objectColor", glm::vec4(0.86f, 0.86f, 0.92f, 1.0f)); // light stone
	m_basicMeshes->DrawBoxMesh();


	// Front fascia
	DrawBox(glm::vec3(3.80f, 0.07f, 0.10f),
		glm::vec3(0, -4.0f, 0),
		P(0.0f, FASCIA_Y, FASCIA_Z),
		TRIM);


	// Porch slab (touches front wall)
	DrawBox(glm::vec3(2.20f, 0.14f, 1.60f),
		glm::vec3(0, -4.0f, 0),
		P(0.00f, PORCH_Y, FRONT_Z - 0.20f),  // slightly back so it tucks under
		STONE);

	// Step 
	DrawBox(glm::vec3(1.70f, 0.12f, 0.75f),
		glm::vec3(0, -4.0f, 0),
		P(0.00f, STEP_Y, FRONT_Z + 0.20f),
		STONE);

	//// MOUNTAIN 
	//DrawMountain(
	//	/*pos*/        glm::vec3(0.0f, 1.3f, -11.5f),
	//	/*baseRadius*/ 7.0f,
	//	/*height*/     4.2f,
	//	/*body*/       glm::vec4(0.62f, 0.82f, 0.78f, 1.0f),
	//	/*cap*/        glm::vec4(0.93f, 0.96f, 1.0f, 1.0f)
	//);


	// snow cap 
	SetTransformations(glm::vec3(2.6f, 0.12f, 2.6f), 0, 0, 0, glm::vec3(0.0f, 3.15f, -11.5f));
	SetShaderColor(0.93f, 0.96f, 1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// TREES
	DrawTree(/*base*/ H + glm::vec3(-3.8f, -1.9f, 1.6f),  /*trunkH*/ 1.0f, /*trunkR*/ 0.18f,
		/*crownH*/ 1.4f, /*crownR*/ 0.9f);

	DrawTree(/*base*/ H + glm::vec3(+3.6f, -1.95f, 1.4f), /*trunkH*/ 0.9f, /*trunkR*/ 0.17f,
		/*crownH*/ 1.2f, /*crownR*/ 0.8f);


	// FENCE — short straight run in front, centered on house
	glm::vec3 fenceStart = H + glm::vec3(-4.5f, -1.85f, 2.25f);
	glm::vec3 fenceDir = glm::normalize(glm::vec3(1, 0, 0));
	DrawFenceLine(fenceStart, fenceDir, /*posts*/ 10, /*spacing*/ 0.95f);


}
