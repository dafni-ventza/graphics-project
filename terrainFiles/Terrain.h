#pragma once;

class Terrain
{
public:

    Terrain( float heightScale = 500.0f, float blockScale = 2.0f );
    virtual ~Terrain();

    void Terminate();
    bool LoadHeightmap( const std::string& filename, unsigned char bitsPerPixel, unsigned int width, unsigned int height );
    bool LoadTexture( const std::string& filename, unsigned int textureStage = 0 );

    // Get the height of the terrain at a position in world space
    float GetHeightAt( const glm::vec3& position );

    void Render();
    // In debug builds, the terrain normals will be rendered.
    void DebugRender();

protected:
    void GenerateIndexBuffer();
    void GenerateNormals();
    // Generates the vertex buffer objects from the 
    // position, normal, texture, and color buffers
    void GenerateVertexBuffers();

    void RenderNormals();

private:
    typedef std::vector<glm::vec3>  PositionBuffer;
    typedef std::vector<glm::vec4>  ColorBuffer;
    typedef std::vector<glm::vec3>  NormalBuffer;
    typedef std::vector<glm::vec2>  TexCoordBuffer;
    typedef std::vector<GLuint>     IndexBuffer;

    PositionBuffer  m_PositionBuffer;
    ColorBuffer     m_ColorBuffer;
    NormalBuffer    m_NormalBuffer;
    TexCoordBuffer  m_Tex0Buffer;
    IndexBuffer     m_IndexBuffer;

    // ID's for the VBO's
    GLuint  m_GLVertexBuffer;
    GLuint  m_GLNormalBuffer;
    GLuint  m_GLColorBuffer;
    GLuint  m_GLTex0Buffer;
    GLuint  m_GLTex1Buffer;
    GLuint  m_GLTex2Buffer;
    GLuint  m_GLIndexBuffer;

    static const unsigned int m_uiNumTextures = 3;
    GLuint  m_GLTextures[m_uiNumTextures];

    glm::mat4x4 m_LocalToWorldMatrix;
    glm::mat4x4 m_InverseLocalToWorldMatrix;

    // The dimensions of the heightmap texture
    glm::uvec2 m_HeightmapDimensions;

    // The height-map value will be multiplied by this value
    // before it is assigned to the vertex's Y-coordinate
    float   m_fHeightScale;
    // The vertex's X and Z coordinates will be multiplied by this
    // for each step when building the terrain
    float   m_fBlockScale;
};