#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"
#include "ObjParser.h"

#include <imgui.h>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{

	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AssembleProgram( m_programID, "Shaders/Vert_PosNormTex.vert", "Shaders/Frag_Lighting_w_Spot.frag" );
}

void CMyApp::CleanShaders()
{
	glDeleteProgram( m_programID );
}

void CMyApp::InitGeometry()
{

	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{ 0, offsetof( Vertex, position ), 3, GL_FLOAT },
		{ 1, offsetof( Vertex, normal   ), 3, GL_FLOAT },
		{ 2, offsetof( Vertex, texcoord ), 2, GL_FLOAT },
	};


	MeshObject<Vertex> quadMeshCPU;

	quadMeshCPU.vertexArray = 
	{
		{ glm::vec3( -1, -1, 0 ),glm::vec3( 0.0, 0.0, 1.0 ), glm::vec2( 0.0, 0.0 ) },
		{ glm::vec3(  1, -1, 0 ),glm::vec3( 0.0, 0.0, 1.0 ), glm::vec2( 1.0, 0.0 ) },
		{ glm::vec3( -1,  1, 0 ),glm::vec3( 0.0, 0.0, 1.0 ), glm::vec2( 0.0, 1.0 ) },
		{ glm::vec3(  1,  1, 0 ),glm::vec3( 0.0, 0.0, 1.0 ), glm::vec2( 1.0, 1.0 ) }
	};

	quadMeshCPU.indexArray =
	{
		0, 1, 2,
		1, 3, 2
	};

	m_quadGPU = CreateGLObjectFromMesh( quadMeshCPU, vertexAttribList );


	MeshObject<Vertex> suzanneMeshCPU = ObjParser::parse("Assets/Suzanne.obj");

	m_SuzanneGPU = CreateGLObjectFromMesh( suzanneMeshCPU, vertexAttribList );


	MeshObject<Vertex> sphereMeshCPU = ObjParser::parse("Assets/MarbleBall.obj");

	m_sphereGPU = CreateGLObjectFromMesh( sphereMeshCPU, vertexAttribList );
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject( m_quadGPU );
	CleanOGLObject( m_SuzanneGPU );
	CleanOGLObject( m_sphereGPU );
}

void CMyApp::InitTextures()
{
	glGenTextures( 1, &m_woodTextureID );
	TextureFromFile( m_woodTextureID, "Assets/Wood_Table_Texture.png" );
	SetupTextureSampling( GL_TEXTURE_2D, m_woodTextureID );

	glGenTextures( 1, &m_SuzanneTextureID );
	TextureFromFile( m_SuzanneTextureID, "Assets/wood.jpg" );
	SetupTextureSampling( GL_TEXTURE_2D, m_SuzanneTextureID );

	glGenTextures( 1, &m_sphereTextureID );
	TextureFromFile( m_sphereTextureID, "Assets/MarbleBall.png" );
	SetupTextureSampling( GL_TEXTURE_2D, m_sphereTextureID );
}

void CMyApp::CleanTextures()
{
	glDeleteTextures( 1, &m_woodTextureID );
	glDeleteTextures( 1, &m_SuzanneTextureID );
	glDeleteTextures( 1, &m_sphereTextureID );
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();

	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);   

	glEnable(GL_DEPTH_TEST); 

	m_camera.SetView(
		glm::vec3(0.0, 7.0, 7.0),
		glm::vec3(0.0, 0.0, 0.0),  
		glm::vec3(0.0, 1.0, 0.0));  

	m_cameraManipulator.SetCamera( &m_camera );

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

void CMyApp::Update( const SUpdateInfo& updateInfo )
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

	m_cameraManipulator.Update( updateInfo.DeltaTimeInSec );


	if ( m_IsLampOn ) 
	{
		glClearColor(0.125f, 0.25f, 0.5f, 1.0f);
		m_lightPos = glm::vec4( m_lampPos, 0.0f );
		m_La = glm::vec3( 0.125f );
		m_Ld = m_Ls = m_lampColor;
	}
	else
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		m_La = glm::vec3( 0.0125f );

		if ( m_IsBurglarIn )
		{
			m_lightPos = glm::vec4( m_spotPos, 1.0f );
			m_Ld = m_Ls = m_lampColor;
			m_spotCosCutoff = cosf( m_spotCutoffAngle );
		}
		else  
		{
			m_lightPos = glm::vec4( m_bugPos, 1.0f );
			m_Ld = m_Ls = BUG_COLOR;
		}
	}
}

void CMyApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray( m_quadGPU.vaoID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_woodTextureID );
	
	glUseProgram( m_programID );

	glUniformMatrix4fv( ul("viewProj"), 1, GL_FALSE, glm::value_ptr( m_camera.GetViewProj() ) );

	glm::mat4 matWorld = glm::identity<glm::mat4>();

	matWorld = glm::translate( matWorld, TABLE_POS ) 
			 * glm::rotate( glm::half_pi<float>(), glm::vec3( -1.0f,0.0,0.0) )
		     * glm::scale( matWorld, glm::vec3( TABLE_SIZE ) );


	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	glUniform3fv( ul( "cameraPos" ), 1, glm::value_ptr( m_camera.GetEye() ) );
	glUniform4fv( ul( "lightPos" ),  1, glm::value_ptr( m_lightPos ) );

	glUniform3fv( ul( "La" ),		 1, glm::value_ptr( m_La ) );
	glUniform3fv( ul( "Ld" ),		 1, glm::value_ptr( m_Ld ) );
	glUniform3fv( ul( "Ls" ),		 1, glm::value_ptr( m_Ls ) );

	glUniform1f( ul( "lightConstantAttenuation"	 ), m_lightConstantAttenuation );
	glUniform1f( ul( "lightLinearAttenuation"	 ), m_lightLinearAttenuation   );
	glUniform1f( ul( "lightQuadraticAttenuation" ), m_lightQuadraticAttenuation);

	glUniform1i( ul( "isSpotLight" ), !m_IsLampOn && m_IsBurglarIn );
	glUniform3fv( ul( "spotDirection" ), 1, glm::value_ptr( m_spotDirection ) );
	glUniform1f( ul( "spotCosCutoff" ), m_spotCosCutoff );
	glUniform1f( ul( "spotExponent" ), m_spotExponent );

	glUniform3fv( ul( "Ka" ),		 1, glm::value_ptr( m_Ka ) );
	glUniform3fv( ul( "Kd" ),		 1, glm::value_ptr( m_Kd ) );
	glUniform3fv( ul( "Ks" ),		 1, glm::value_ptr( m_Ks ) );

	glUniform1f( ul( "Shininess" ),	m_Shininess );

	glUniform1i( ul( "texImage" ), 0 );

	glDrawElements( GL_TRIANGLES,    
					m_quadGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );


	glBindVertexArray( m_SuzanneGPU.vaoID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_SuzanneTextureID );

	matWorld = glm::translate( SUZANNE_POS );

	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );
	
	glDrawElements( GL_TRIANGLES,    
					m_SuzanneGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );


	glBindVertexArray( m_sphereGPU.vaoID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_sphereTextureID );

	matWorld = glm::translate( SPHERE_POS );

	glUniformMatrix4fv( ul( "world" ),    1, GL_FALSE, glm::value_ptr( matWorld ) );
	glUniformMatrix4fv( ul( "worldIT" ),  1, GL_FALSE, glm::value_ptr( glm::transpose( glm::inverse( matWorld ) ) ) );

	glDrawElements( GL_TRIANGLES,    
					m_sphereGPU.count,			 
					GL_UNSIGNED_INT,
					nullptr );

	glUseProgram( 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glBindVertexArray( 0 );
}

void CMyApp::RenderGUI()
{
	if ( ImGui::Begin( "Lighting settings" ) )
	{
		ImGui::Checkbox("Is lamp on?", &m_IsLampOn );
		ImGui::SeparatorText("Settings");
		
		ImGui::InputFloat("Shininess", &m_Shininess, 0.1f, 1.0f, "%.1f" );
		static float Kaf = 1.0f;
		static float Kdf = 1.0f;
		static float Ksf = 1.0f;
		if ( ImGui::SliderFloat( "Ka", &Kaf, 0.0f, 1.0f ) )
		{
			m_Ka = glm::vec3( Kaf );
		}
		if ( ImGui::SliderFloat( "Kd", &Kdf, 0.0f, 1.0f ) )
		{
			m_Kd = glm::vec3( Kdf );
		}
		if ( ImGui::SliderFloat( "Ks", &Ksf, 0.0f, 1.0f ) )
		{
			m_Ks = glm::vec3( Ksf );
		}

		if ( m_IsLampOn || m_IsBurglarIn )
		{
			static glm::vec2 lampPosXZ = glm::vec2( 0.0f );
			lampPosXZ = glm::vec2( m_lampPos.x, m_lampPos.z );
			if ( ImGui::SliderFloat2( "Lamp Position XZ", glm::value_ptr( lampPosXZ ), -1.0f, 1.0f ) )
			{
				float lampPosL2 = lampPosXZ.x * lampPosXZ.x + lampPosXZ.y * lampPosXZ.y;
				if ( lampPosL2 > 1.0f ) 
				{
					lampPosXZ /= sqrtf( lampPosL2 );
					lampPosL2 = 1.0f;
				}

				m_lampPos.x = lampPosXZ.x;
				m_lampPos.z = lampPosXZ.y;
				m_lampPos.y = sqrtf( 1.0f - lampPosL2 );
			}
			ImGui::LabelText( "Lamp Position Y", "%f", m_lampPos.y );
			ImGui::ColorEdit3( "Lamp color", glm::value_ptr( m_lampColor ), ImGuiColorEditFlags_Float );
		}
		if ( !m_IsLampOn )
		{
			ImGui::Checkbox("Burglars in?", &m_IsBurglarIn );

			ImGui::SliderFloat( "Constant Attenuation", &m_lightConstantAttenuation, 0.001f, 2.0f );
			ImGui::SliderFloat( "Linear Attenuation", &m_lightLinearAttenuation, 0.001f, 2.0f );
			ImGui::SliderFloat( "Quadratic Attenuation", &m_lightQuadraticAttenuation, 0.001f, 2.0f );

			if ( m_IsBurglarIn )
			{
				ImGui::SliderFloat3( "Spot light position", glm::value_ptr(m_spotPos), -TABLE_SIZE, TABLE_SIZE);
				static glm::vec2 spotDirXZ = glm::vec2( 0.0f );
				spotDirXZ = glm::vec2( m_spotDirection.x,m_spotDirection.z );
				if ( ImGui::SliderFloat2( "Spot light direction XZ", glm::value_ptr(spotDirXZ), -1.0f,1.0f))
				{
					float spotDirL2 = spotDirXZ.x * spotDirXZ.x + spotDirXZ.y * spotDirXZ.y;
					if ( spotDirL2 > 1.0f ) 
					{
						spotDirXZ /= sqrtf( spotDirL2 );
						spotDirL2 = 1.0f;
					}
					m_spotDirection = glm::vec3( spotDirXZ.x, -sqrtf( 1.0f - spotDirL2 ), spotDirXZ.y);

				}

				if ( ImGui::SliderAngle( "Spot light cutoff angle", &m_spotCutoffAngle, 0.0f, 90.0f ) )
				{
					m_spotCosCutoff = cosf( m_spotCutoffAngle );
				}
				ImGui::InputFloat( "Spot light exponent", &m_spotExponent, 1.0f, 10.0f );
			}
			else
			{
				ImGui::SliderFloat3( "Bug Position", glm::value_ptr( m_bugPos ), -TABLE_SIZE, TABLE_SIZE );
			}
		}
		
	}

	ImGui::End();
}

GLint CMyApp::ul( const char* uniformName ) noexcept
{
	GLuint programID = 0;

	glGetIntegerv( GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>( &programID ) );

	return glGetUniformLocation( programID, uniformName );
}


void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{	
	if ( key.repeat == 0 ) 
	{
		if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL )
		{
			CleanShaders();
			InitShaders();
		}
		if ( key.keysym.sym == SDLK_F1 )
		{
			GLint polygonModeFrontAndBack[ 2 ] = {};
			glGetIntegerv( GL_POLYGON_MODE, polygonModeFrontAndBack ); 
			GLenum polygonMode = ( polygonModeFrontAndBack[ 0 ] != GL_FILL ? GL_FILL : GL_LINE ); 
			glPolygonMode( GL_FRONT_AND_BACK, polygonMode ); 
		}
	}
	m_cameraManipulator.KeyboardDown( key );
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp( key );
}


void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove( mouse );
}


void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{
}


void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel( wheel );
}


void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_camera.SetAspect( static_cast<float>(_w) / _h );
}


void CMyApp::OtherEvent( const SDL_Event& ev )
{

}
