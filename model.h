#ifndef _MESH_H
#define _MESH_H

typedef struct Vertex{
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
} Vertex;

typedef struct Texture {
    GLuint id;
    char* type;
    struct aiString directory;
} Texture;

typedef struct Mesh {
    Vertex* vertices;
    GLuint* indices;
    Texture* textures;
    GLsizeiptr v_size, i_size, t_size;
    GLuint VAO, VBO, EBO;
} Mesh;

typedef struct Model {
    Mesh* meshes;
    GLsizeiptr m_size;
    char* directory;
    Texture* textures_loaded;
    GLsizeiptr tl_size;
} Model;

void initMesh(Mesh* mesh, Vertex* vertices, GLsizeiptr vertices_size, GLuint* indices, GLsizeiptr indices_size, Texture* textures, GLsizeiptr textures_size);
void setupMesh(Mesh* mesh);
void drawMesh(Mesh* mesh, GLuint shader);

void drawModel(Model* model, GLuint shader);
void loadModel(Model* model, GLchar* path);
void processNodeModel(Model* model, struct aiNode* node, const struct aiScene* scene, GLsizeiptr* last_index);
void processMeshModel(Model* model, struct aiMesh* mesh, const struct aiScene* scene, GLsizeiptr* last_index);
void loadMaterialModel(Model* model, struct aiMaterial* mat, enum aiTextureType type, char* typeName, Mesh* mesh, GLsizeiptr index);
GLuint textureFromFile(const char* path, char* dir);
GLuint loadTexture(const char* path);
char* getDirectory(const char* path, char delimiter);

#endif