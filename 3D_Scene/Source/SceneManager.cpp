///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st,
// 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace {
const char *g_ModelName = "model";
const char *g_ColorValueName = "objectColor";
const char *g_TextureValueName = "objectTexture";
const char *g_UseTextureName = "bUseTexture";
const char *g_UseLightingName = "bUseLighting";
} // namespace

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager) {
  m_pShaderManager = pShaderManager;
  m_basicMeshes = new ShapeMeshes();
  m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager() {
  m_pShaderManager = NULL;
  delete m_basicMeshes;
  m_basicMeshes = NULL;
  DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char *filename, std::string tag) {
  int width = 0;
  int height = 0;
  int colorChannels = 0;
  GLuint textureID = 0;

  // indicate to always flip images vertically when loaded
  stbi_set_flip_vertically_on_load(true);

  // try to parse the image data from the specified image file
  unsigned char *image =
      stbi_load(filename, &width, &height, &colorChannels, 0);

  // if the image was successfully read from the image file
  if (image) {
    std::cout << "Successfully loaded image:" << filename << ", width:" << width
              << ", height:" << height << ", channels:" << colorChannels
              << std::endl;

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
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, image);
    // if the loaded image is in RGBA format - it supports transparency
    else if (colorChannels == 4)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, image);
    else {
      std::cout << "Not implemented to handle image with " << colorChannels
                << " channels" << std::endl;
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
void SceneManager::BindGLTextures() {
  for (int i = 0; i < m_loadedTextures; i++) {
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
void SceneManager::DestroyGLTextures() {
  for (int i = 0; i < m_loadedTextures; i++) {
    glDeleteTextures(1, &m_textureIDs[i].ID);
  }
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag) {
  int textureID = -1;
  int index = 0;
  bool bFound = false;

  while ((index < m_loadedTextures) && (bFound == false)) {
    if (m_textureIDs[index].tag.compare(tag) == 0) {
      textureID = m_textureIDs[index].ID;
      bFound = true;
    } else
      index++;
  }

  return (textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag) {
  int textureSlot = -1;
  int index = 0;
  bool bFound = false;

  while ((index < m_loadedTextures) && (bFound == false)) {
    if (m_textureIDs[index].tag.compare(tag) == 0) {
      textureSlot = index;
      bFound = true;
    } else
      index++;
  }

  return (textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL &material) {
  if (m_objectMaterials.size() == 0) {
    return (false);
  }

  size_t index = 0;
  bool bFound = false;
  while ((index < m_objectMaterials.size()) && (bFound == false)) {
    if (m_objectMaterials[index].tag.compare(tag) == 0) {
      bFound = true;
      material.ambientColor = m_objectMaterials[index].ambientColor;
      material.ambientStrength = m_objectMaterials[index].ambientStrength;
      material.diffuseColor = m_objectMaterials[index].diffuseColor;
      material.specularColor = m_objectMaterials[index].specularColor;
      material.shininess = m_objectMaterials[index].shininess;
    } else {
      index++;
    }
  }

  return (bFound);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(glm::vec3 scaleXYZ,
                                      float XrotationDegrees,
                                      float YrotationDegrees,
                                      float ZrotationDegrees,
                                      glm::vec3 positionXYZ) {
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
  rotationX =
      glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
  rotationY =
      glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
  rotationZ =
      glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
  // set the translation value in the transform buffer
  translation = glm::translate(positionXYZ);

  modelView = translation * rotationX * rotationY * rotationZ * scale;

  if (NULL != m_pShaderManager) {
    m_pShaderManager->setMat4Value(g_ModelName, modelView);
  }
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(float redColorValue, float greenColorValue,
                                  float blueColorValue, float alphaValue) {
  // variables for this method
  glm::vec4 currentColor;

  currentColor.r = redColorValue;
  currentColor.g = greenColorValue;
  currentColor.b = blueColorValue;
  currentColor.a = alphaValue;

  if (NULL != m_pShaderManager) {
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
void SceneManager::SetShaderTexture(std::string textureTag) {
  if (NULL != m_pShaderManager) {
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
void SceneManager::SetTextureUVScale(float u, float v) {
  if (NULL != m_pShaderManager) {
    m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
  }
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag) {
  if (m_objectMaterials.size() > 0) {
    OBJECT_MATERIAL material;
    bool bReturn = false;

    bReturn = FindMaterial(materialTag, material);
    if (bReturn == true) {
      m_pShaderManager->setVec3Value("material.ambientColor",
                                     material.ambientColor);
      m_pShaderManager->setFloatValue("material.ambientStrength",
                                      material.ambientStrength);
      m_pShaderManager->setVec3Value("material.diffuseColor",
                                     material.diffuseColor);
      m_pShaderManager->setVec3Value("material.specularColor",
                                     material.specularColor);
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

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects in the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials() {
  OBJECT_MATERIAL goldMaterial;
  goldMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.1f);
  goldMaterial.ambientStrength = 0.4f;
  goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
  goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
  goldMaterial.shininess = 22.0f;
  goldMaterial.tag = "gold";
  m_objectMaterials.push_back(goldMaterial);

  OBJECT_MATERIAL cementMaterial;
  cementMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
  cementMaterial.ambientStrength = 0.2f;
  cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
  cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
  cementMaterial.shininess = 0.5f;
  cementMaterial.tag = "cement";
  m_objectMaterials.push_back(cementMaterial);

  OBJECT_MATERIAL woodMaterial;
  woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
  woodMaterial.ambientStrength = 0.2f;
  woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
  woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
  woodMaterial.shininess = 0.3f;
  woodMaterial.tag = "wood";
  m_objectMaterials.push_back(woodMaterial);

  OBJECT_MATERIAL tileMaterial;
  tileMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
  tileMaterial.ambientStrength = 0.3f;
  tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
  tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
  tileMaterial.shininess = 25.0f;
  tileMaterial.tag = "tile";
  m_objectMaterials.push_back(tileMaterial);

  OBJECT_MATERIAL glassMaterial;
  glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
  glassMaterial.ambientStrength = 0.3f;
  glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
  glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
  glassMaterial.shininess = 85.0f;
  glassMaterial.tag = "glass";
  m_objectMaterials.push_back(glassMaterial);

  OBJECT_MATERIAL clayMaterial;
  clayMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
  clayMaterial.ambientStrength = 0.3f;
  clayMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
  clayMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.4f);
  clayMaterial.shininess = 0.5f;
  clayMaterial.tag = "clay";
  m_objectMaterials.push_back(clayMaterial);

  OBJECT_MATERIAL stainlessMaterial;
  stainlessMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
  stainlessMaterial.ambientStrength = 0.3f;
  stainlessMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
  stainlessMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);
  stainlessMaterial.shininess = 2.0f;
  stainlessMaterial.tag = "stainless";
  m_objectMaterials.push_back(stainlessMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is used to add and configure the various light
 *  sources that will add more realism to the 3D scene.
 ***********************************************************/
void SceneManager::SetupSceneLights() {
  // Light 0 (Key Light - Front Right)
  m_pShaderManager->setVec3Value("lightSources[0].position", 5.0f, 4.0f, 5.0f);
  m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.02f, 0.02f,
                                 0.02f);
  m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.7f, 0.7f,
                                 0.7f);
  m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.6f, 0.6f,
                                 0.6f);
  m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
  m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.8f);

  // Light 1 (Fill Light - Front Left) - Colored Light
  m_pShaderManager->setVec3Value("lightSources[1].position", -5.0f, 3.0f, 4.0f);
  m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.02f);
  m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.1f, 0.2f,
                                 0.6f);
  m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.1f, 0.2f,
                                 0.6f);
  m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0f);
  m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.3f);

  // Light 2 (Back/Rim Light - Behind) - Bright Purple Light
  m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 4.0f, -4.0f);
  m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.01f, 0.0f, 0.01f);
  m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.9f, 0.0f, 0.9f);
  m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.9f, 0.0f, 0.9f);
  m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 32.0f);
  m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 1.0f);

  // Light 3 (Explicitly Disabled)
  m_pShaderManager->setVec3Value("lightSources[3].position", 0.0f, 0.0f, 0.0f);
  m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.0f, 0.0f,
                                 0.0f);
  m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.0f, 0.0f,
                                 0.0f);
  m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f,
                                 0.0f);
  m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
  m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);

  m_pShaderManager->setBoolValue("bUseLighting", true);
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene() {
  // only one instance of a particular mesh needs to be
  // loaded in memory no matter how many times it is drawn
  // in the rendered 3D scene

  DefineObjectMaterials();
  SetupSceneLights();

  m_basicMeshes->LoadPlaneMesh();
  m_basicMeshes->LoadCylinderMesh();
  m_basicMeshes->LoadTorusMesh();
  m_basicMeshes->LoadSphereMesh();
  m_basicMeshes->LoadBoxMesh();
  m_basicMeshes->LoadConeMesh();

  // load the texture images
  CreateGLTexture("textures/rusticwood.jpg", "desk");
  CreateGLTexture("textures/stainless.jpg", "mug_body");
  CreateGLTexture("textures/stainless_end.jpg", "mug_handle");
  CreateGLTexture("textures/stainedglass.jpg", "marble_ball");
  CreateGLTexture("textures/tilesf2.jpg", "coaster");
  CreateGLTexture("textures/abstract.jpg", "notebook");
  CreateGLTexture("textures/gold-seamless-texture.jpg",
                  "pencil_body");
  CreateGLTexture("textures/breadcrust.jpg", "pencil_tip");
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene() {
  // Bind loaded OpenGL textures to slots in memory
  BindGLTextures();

  // declare the variables for the transformations
  glm::vec3 scaleXYZ;
  float XrotationDegrees = 0.0f;
  float YrotationDegrees = 0.0f;
  float ZrotationDegrees = 0.0f;
  glm::vec3 positionXYZ;

  /*** Set needed transformations before drawing the basic mesh.  ***/
  /*** This same ordering of code should be used for transforming ***/
  /*** and drawing all the basic 3D shapes.
   * ***/
  /******************************************************************/
  // set the XYZ scale for the mesh
  scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

  // set the XYZ rotation for the mesh
  XrotationDegrees = 0.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = 0.0f;

  // set the XYZ position for the mesh
  positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

  // set the transformations into memory to be used on the drawn meshes
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);

  // Apply tiled wood texture to the desk plane
  SetShaderTexture("desk");
  SetTextureUVScale(4.0f, 4.0f);
  SetShaderMaterial("wood");

  // draw the mesh with transformation values
  m_basicMeshes->DrawPlaneMesh();
  /****************************************************************/

  // --- PLANAR SHADOWS FROM KEY LIGHT ---
  // Disable lighting and texturing for the shadows to render flat black
  m_pShaderManager->setBoolValue("bUseLighting", false);
  SetShaderColor(0.0f, 0.0f, 0.0f, 0.45f); // 45% opacity black for shadow

  // Create the parallel projection shadow matrix (keeps w = 1.0f to prevent
  // camera projection distortion)
  glm::vec3 lightPos = glm::vec3(5.0f, 4.0f, 5.0f); // Key Light position
  float lx = lightPos.x;
  float ly = lightPos.y;
  float lz = lightPos.z;

  glm::mat4 shadowMat(1.0f);
  shadowMat[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
  shadowMat[1] = glm::vec4(-lx / ly, 0.0f, -lz / ly, 0.0f);
  shadowMat[2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
  shadowMat[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

  // Translate slightly above the desk plane to avoid z-fighting
  glm::mat4 shadowOffset = glm::translate(glm::vec3(0.0f, 0.005f, 0.0f));
  glm::mat4 shadowProj = shadowOffset * shadowMat;

  // Temporarily disable depth testing to completely eliminate depth-buffer
  // z-fighting artifacts
  glDisable(GL_DEPTH_TEST);

  // 1. Render Cylinder (Mug Body) Shadow
  glm::mat4 cylinderScale = glm::scale(glm::vec3(1.5f, 3.0f, 1.5f));
  glm::mat4 cylinderModel = cylinderScale;
  m_pShaderManager->setMat4Value("model", shadowProj * cylinderModel);
  m_basicMeshes->DrawCylinderMesh();

  // 2. Render Torus (Mug Handle) Shadow
  glm::mat4 torusScale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
  glm::mat4 torusRotZ =
      glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 torusTrans = glm::translate(glm::vec3(1.5f, 1.5f, 0.0f));
  glm::mat4 torusModel = torusTrans * torusRotZ * torusScale;
  m_pShaderManager->setMat4Value("model", shadowProj * torusModel);
  m_basicMeshes->DrawHalfTorusMesh();

  // 3. Render Coaster (Box) Shadow
  glm::mat4 coasterScale = glm::scale(glm::vec3(2.5f, 0.1f, 2.5f));
  glm::mat4 coasterRotY =
      glm::rotate(glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 coasterTrans = glm::translate(glm::vec3(-3.0f, 0.0f, -1.0f));
  glm::mat4 coasterModel = coasterTrans * coasterRotY * coasterScale;
  m_pShaderManager->setMat4Value("model", shadowProj * coasterModel);
  m_basicMeshes->DrawBoxMesh();

  // 4. Render Sphere (Marble Ball) Shadow
  glm::mat4 sphereScale = glm::scale(glm::vec3(0.75f, 0.75f, 0.75f));
  glm::mat4 sphereTrans = glm::translate(glm::vec3(-3.0f, 0.80f, -1.0f));
  glm::mat4 sphereModel = sphereTrans * sphereScale;
  m_pShaderManager->setMat4Value("model", shadowProj * sphereModel);
  m_basicMeshes->DrawSphereMesh();

  // 5. Render Box (Notebook) Shadow
  glm::mat4 boxScale = glm::scale(glm::vec3(1.5f, 1.5f, 2.2f));
  glm::mat4 boxRotY =
      glm::rotate(glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 boxTrans = glm::translate(glm::vec3(3.0f, 0.75f, -1.5f));
  glm::mat4 boxModel = boxTrans * boxRotY * boxScale;
  m_pShaderManager->setMat4Value("model", shadowProj * boxModel);
  m_basicMeshes->DrawBoxMesh();

  // 6. Render Pencil (Cylinder + Cone) Shadow
  // Pencil Body
  glm::mat4 pencilBodyScale = glm::scale(glm::vec3(0.06f, 1.5f, 0.06f));
  glm::mat4 pencilBodyRotX =
      glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 pencilBodyTrans = glm::translate(glm::vec3(1.5f, 0.06f, 1.4f));
  glm::mat4 pencilBodyModel =
      pencilBodyTrans * pencilBodyRotX * pencilBodyScale;
  m_pShaderManager->setMat4Value("model", shadowProj * pencilBodyModel);
  m_basicMeshes->DrawCylinderMesh();

  // Pencil Tip
  glm::mat4 pencilTipScale = glm::scale(glm::vec3(0.06f, 0.3f, 0.06f));
  glm::mat4 pencilTipRotX =
      glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 pencilTipTrans = glm::translate(glm::vec3(1.5f, 0.06f, 1.4f));
  glm::mat4 pencilTipModel = pencilTipTrans * pencilTipRotX * pencilTipScale;
  m_pShaderManager->setMat4Value("model", shadowProj * pencilTipModel);
  m_basicMeshes->DrawConeMesh();

  // Re-enable depth testing and lighting for subsequent meshes
  glEnable(GL_DEPTH_TEST);
  m_pShaderManager->setBoolValue("bUseLighting", true);

  // --- COFFEE MUG BODY (Cylinder) ---
  // set the XYZ scale for the mesh
  scaleXYZ = glm::vec3(1.5f, 3.0f, 1.5f);

  // set the XYZ rotation for the mesh
  XrotationDegrees = 0.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = 0.0f;

  // set the XYZ position for the mesh (resting on the desk)
  positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

  // set the transformations into memory to be used on the drawn meshes
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);

  // Apply circular-brushed-gold texture to the mug body
  SetShaderTexture("mug_body");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("stainless");

  // draw the mesh with transformation values
  m_basicMeshes->DrawCylinderMesh();

  // --- COFFEE MUG HANDLE (Torus) ---
  // set the XYZ scale for the mesh
  scaleXYZ = glm::vec3(1.0f, 1.0f, 1.0f);

  // set the XYZ rotation for the mesh (rotate upright to form a handle)
  XrotationDegrees = 0.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = -90.0f;

  // set the XYZ position for the mesh (move to the side and up)
  positionXYZ = glm::vec3(1.5f, 1.5f, 0.0f);

  // set the transformations into memory to be used on the drawn meshes
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);

  // Apply gold-seamless texture to the handle (Option 2: flipped UV scale
  // horizontally)
  SetShaderTexture("mug_handle");
  SetTextureUVScale(-1.0f, 1.0f);
  SetShaderMaterial("stainless");

  // draw the mesh with transformation values
  m_basicMeshes->DrawHalfTorusMesh();
  /****************************************************************/

  // --- COASTER (Box) ---
  scaleXYZ = glm::vec3(2.5f, 0.1f, 2.5f);
  XrotationDegrees = 0.0f;
  YrotationDegrees = 15.0f;
  ZrotationDegrees = 0.0f;
  positionXYZ = glm::vec3(-3.0f, 0.0f, -1.0f);
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);
  SetShaderTexture("coaster");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("tile");
  m_basicMeshes->DrawBoxMesh();
  /****************************************************************/

  // --- MARBLE BALL (Sphere) ---
  scaleXYZ = glm::vec3(0.75f, 0.75f, 0.75f);
  XrotationDegrees = 0.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = 0.0f;
  positionXYZ = glm::vec3(-3.0f, 0.80f, -1.0f);
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);
  SetShaderTexture("marble_ball");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("glass");
  m_basicMeshes->DrawSphereMesh();
  /****************************************************************/

  // --- NOTEBOOK / BOX (Box) ---
  scaleXYZ = glm::vec3(1.5f, 1.5f, 2.2f);
  XrotationDegrees = 0.0f;
  YrotationDegrees = -25.0f;
  ZrotationDegrees = 0.0f;
  positionXYZ = glm::vec3(3.0f, 0.75f, -1.5f);
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);
  SetShaderTexture("notebook");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("clay");
  m_basicMeshes->DrawBoxMesh();
  /****************************************************************/

  // --- PENCIL BODY (Cylinder) ---
  scaleXYZ = glm::vec3(0.06f, 1.5f, 0.06f);
  XrotationDegrees = 90.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = 0.0f;
  positionXYZ = glm::vec3(1.5f, 0.06f, 1.4f);
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);
  SetShaderTexture("pencil_body");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("gold");
  m_basicMeshes->DrawCylinderMesh();
  /****************************************************************/

  // --- PENCIL TIP (Cone) ---
  scaleXYZ = glm::vec3(0.06f, 0.3f, 0.06f);
  XrotationDegrees = -90.0f;
  YrotationDegrees = 0.0f;
  ZrotationDegrees = 0.0f;
  positionXYZ = glm::vec3(1.5f, 0.06f, 1.4f);
  SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees,
                     ZrotationDegrees, positionXYZ);
  SetShaderTexture("pencil_tip");
  SetTextureUVScale(1.0f, 1.0f);
  SetShaderMaterial("clay");
  m_basicMeshes->DrawConeMesh();
  /****************************************************************/
}
