#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "GLUtils.hpp"
#include "Camera.h"
#include "CameraManipulator.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f; 
	float DeltaTimeInSec   = 0.0f; 
};

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update( const SUpdateInfo& );
	void Render();
	void RenderGUI();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);

	void OtherEvent( const SDL_Event& );
protected:
	void SetupDebugCallback();

	float m_ElapsedTimeInSec = 0.0f;

	bool m_IsLampOn = true;
	bool m_IsBurglarIn = false;

	static constexpr float     TABLE_SIZE = 5.0f;
	static constexpr glm::vec3 TABLE_POS = glm::vec3( 0.0f, 0.0f, 0.0f );

	static constexpr glm::vec3 SUZANNE_POS = glm::vec3( 2.0f, 1.0f, 2.0f );

	static constexpr glm::vec3 SPHERE_POS = glm::vec3( -2.0f, 1.0f, -2.0f );

	Camera m_camera;
	CameraManipulator m_cameraManipulator;


	glm::vec3 m_lampPos = glm::vec3( 0.0f, 1.0f, 0.0f );
	glm::vec3 m_lampColor = glm::vec3( 1.0f, 1.0f, 1.0f );

	glm::vec3 m_bugPos = glm::vec3( 0.0f, 1.0f, 0.0f );
	static constexpr glm::vec3 BUG_COLOR = glm::vec3( 0.53f, 1.0f, 0.3f );

	glm::vec3 m_spotPos = glm::vec3( 0.0f, 1.0f, 0.0f );

	float m_spotCutoffAngle = glm::pi<float>();

	static GLint ul( const char* uniformName ) noexcept;

	GLuint m_programID = 0; 


	glm::vec4 m_lightPos = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );

	glm::vec3 m_La = glm::vec3(0.0, 0.0, 0.0 );
	glm::vec3 m_Ld = glm::vec3(1.0, 1.0, 1.0 );
	glm::vec3 m_Ls = glm::vec3(1.0, 1.0, 1.0 );

	float m_lightConstantAttenuation    = 1.0;
	float m_lightLinearAttenuation      = 0.0;
	float m_lightQuadraticAttenuation   = 0.0;

	float     m_spotCosCutoff = 0.0;
	float     m_spotExponent  = 0.0;
	glm::vec3 m_spotDirection = glm::vec3(0.0, -1.0, 0.0);


	glm::vec3 m_Ka = glm::vec3( 1.0 );
	glm::vec3 m_Kd = glm::vec3( 1.0 );
	glm::vec3 m_Ks = glm::vec3( 1.0 );

	float m_Shininess = 1.0;

	void InitShaders();
	void CleanShaders();


	OGLObject m_quadGPU = {};
	OGLObject m_SuzanneGPU = {};
	OGLObject m_sphereGPU = {};

	void InitGeometry();
	void CleanGeometry();


	GLuint m_woodTextureID = 0;
	GLuint m_SuzanneTextureID = 0;
	GLuint m_sphereTextureID = 0;

	void InitTextures();
	void CleanTextures();
};

