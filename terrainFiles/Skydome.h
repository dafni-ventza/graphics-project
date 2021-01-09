#pragma once;


// Skydome to render an infinite textures sphere around the camera
class Skydome
{
public:
    explicit Skydome( float fRadius = 1.0f, unsigned int uiSlices = 16, unsigned int uiStacks = 16 );
    virtual ~Skydome();

    void Init();
    void Terminate();


    bool LoadTexture( const std::string& filename );
    void Render();
protected:

private:
    // The OpenGL resources
    GLuint          m_GLTextureID;
    GLUquadricObj*  m_QuadricObj;

    float           m_fRadius;
    unsigned int    m_uiSlices;
    unsigned int    m_uiStacks;

    
};