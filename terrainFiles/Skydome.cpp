#include "TerrainPCH.h"
#include "Skydome.h"

Skydome::Skydome( float fRadius /* = 1.0f */, unsigned int uiSlices /* = 16 */, unsigned int uiStacks /* = 16 */ )
: m_GLTextureID(0)
, m_QuadricObj( NULL )
, m_fRadius( fRadius )
, m_uiSlices( uiSlices )
, m_uiStacks( uiStacks )
{}

Skydome::~Skydome()
{
    Terminate();
}

void Skydome::Terminate()
{
    if ( m_GLTextureID != 0 )
    {
        glDeleteTextures( 1, &m_GLTextureID );
        m_GLTextureID = 0;
    }
    if ( m_QuadricObj != NULL )
    {
        gluDeleteQuadric( m_QuadricObj );
        m_QuadricObj = NULL;
    }
}
void Skydome::Init()
{
    Terminate();

    m_QuadricObj = gluNewQuadric();
    assert( m_QuadricObj != NULL );

    gluQuadricDrawStyle( m_QuadricObj, GLU_FILL );
    gluQuadricOrientation( m_QuadricObj, GLU_INSIDE );
    gluQuadricTexture( m_QuadricObj, GLU_TRUE );
}


bool Skydome::LoadTexture( const std::string& filename )
{
    if ( m_GLTextureID != 0 )
    {
        glDeleteTextures(1, &m_GLTextureID );
    }

    m_GLTextureID = SOIL_load_OGL_texture( filename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );

    if ( m_GLTextureID != 0 ) 
    {
        glBindTexture( GL_TEXTURE_2D, m_GLTextureID );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    return ( m_GLTextureID != 0 );
}

void Skydome::Render()
{
    glPushAttrib( GL_ENABLE_BIT );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /*if ( m_pCamera != NULL )
    {
        glm::vec3 cameraRotation = m_pCamera->GetRotation();
        glRotatef( cameraRotation.x, -1.0f, 0.0f, 0.0f );
        glRotatef( cameraRotation.y, 0.0f, -1.0f, 0.0f );
        glRotatef( cameraRotation.z, 0.0f, 0.0f, -1.0f );
    }*/
   
    // Rotate an additional 90 degrees around the local X axis
    glRotatef( 90, 1.0f, 0.0f, 0.0f );

    glDisable( GL_DEPTH_TEST );
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_GLTextureID );

    glColor3f( 1.0f, 1.0f, 1.0f );
    gluSphere( m_QuadricObj, m_fRadius, m_uiSlices, m_uiStacks );

    glBindTexture( GL_TEXTURE_2D, 0 );

    glPopMatrix();

    glPopAttrib();  // Restore the enable bit attributes

}
