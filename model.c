#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <SOIL/SOIL.h>
#define CGLM_DEFINE_PRINTS
#include <cglm/cglm.h>
#include "model.h"
#include "shader.h"
#define nullptr (void*)0

void initMesh(Mesh* mesh, Vertex* vertices, GLsizeiptr v_size, GLuint* indices, GLsizeiptr i_size, Texture* textures, GLsizeiptr t_size) {
    mesh->vertices = vertices;
    mesh->v_size = v_size;
    mesh->indices = indices;
    mesh->i_size = i_size;
    mesh->textures = textures;
    mesh->t_size = t_size;

    setupMesh(mesh);
}

void setupMesh(Mesh* mesh) {
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->v_size, mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh->i_size, mesh->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(sizeof(GLfloat) * 6));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void drawMesh(Mesh* mesh, GLuint shader) {
    GLuint diff_nr = 1, spec_nr = 1;
    for (GLuint i = 0; i < mesh->t_size; i++) {
        glActiveTexture(GL_TEXTURE0+i);
        GLuint current_num = (strcmp(mesh->textures[i].type, "texture_diffuse")) ? spec_nr++ : diff_nr++;
        GLchar* number = (GLchar*) calloc(sizeof(GLchar*), (int)log10((double)current_num)+2);
        sprintf(number, "%d", current_num);
        GLchar* name = (GLchar*) calloc(sizeof(GLchar), strlen(mesh->textures[i].type) + strlen(number) + 11);
        strcpy(name, "material."); strcat(name, mesh->textures[i].type); strcat(name, number);
        glUniform1i(glGetUniformLocation(shader, name), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }
    glUniform1f(glGetUniformLocation(shader, "material.shininess"), 16.0f);

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->i_size, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for (GLuint i = 0; i < mesh->t_size; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void drawModel(Model* model, GLuint shader) {
    for (GLuint i = 0; i < model->m_size; i++)
        drawMesh(&model->meshes[i], shader);
}

void loadModel(Model* model, GLchar* path) {
    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        nqraise("ERROR::ASSIMP", aiGetErrorString());
        return;
    }
    model->m_size = scene->mNumMeshes;
    model->meshes = (Mesh*) malloc(sizeof(Mesh) * model->m_size);

    model->directory = (GLchar*) calloc(sizeof(GLchar), strlen(path)+1);
strcpy(model->directory, getDirectory(path, '/'));

    model->tl_size = 0;
    model->textures_loaded = (Texture*) calloc(sizeof(Texture), scene->mNumMaterials);

    GLsizeiptr index = 0;
    processNodeModel(model, scene->mRootNode, scene, &index);
}

void processNodeModel(Model* model, struct aiNode* node, const struct aiScene* scene, GLsizeiptr* last_index) {
    fflush(stdout);
    for (GLuint i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMeshModel(model, mesh, scene, last_index);
        (*last_index)++;
    }

    for (GLuint i = 0; i < node->mNumChildren; i++) {
        processNodeModel(model, node->mChildren[i], scene, last_index);
    }
}

void processMeshModel(Model* model, struct aiMesh* mesh, const struct aiScene* scene, GLsizeiptr* last_index) {
    Mesh* current_mesh = &model->meshes[*last_index];
    current_mesh->v_size = mesh->mNumVertices;
    current_mesh->vertices = (Vertex*) malloc(sizeof(Mesh) * current_mesh->v_size);
    current_mesh->i_size = mesh->mNumFaces * 3;
    current_mesh->indices = (GLuint*) malloc(sizeof(GLuint) * current_mesh->i_size);
    
    for (GLuint i = 0; i < current_mesh->v_size; i++) {
        glm_vec3_copy((vec3){mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z}, current_mesh->vertices[i].position);
        glm_vec3_copy((vec3){mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z}, current_mesh->vertices[i].normal);
        if (mesh->mTextureCoords[0])
            glm_vec2_copy((vec2){mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}, current_mesh->vertices[i].tex_coords);
        else
            glm_vec2_copy((vec2){0.0f, 0.0f}, current_mesh->vertices[i].tex_coords);
    }

    for (GLuint i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; j++)
            current_mesh->indices[i*face.mNumIndices+j] = face.mIndices[j];
    }

    current_mesh->t_size = 0;
    current_mesh->textures = (Texture*) malloc(0);
    if (mesh->mMaterialIndex >= 0) {
        struct aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        loadMaterialModel(model, material, aiTextureType_DIFFUSE, "texture_diffuse", current_mesh, 0);
        loadMaterialModel(model, material, aiTextureType_SPECULAR, "texture_specular", current_mesh, current_mesh->t_size);
    }

    setupMesh(current_mesh);
}

void loadMaterialModel(Model* model, struct aiMaterial* mat, enum aiTextureType type, char* typeName, Mesh* mesh, GLsizeiptr index) {
    mesh->textures = realloc(mesh->textures, sizeof(Texture) * (mesh->t_size+aiGetMaterialTextureCount(mat, type)));
    mesh->t_size += aiGetMaterialTextureCount(mat, type);
    for (GLuint i = 0; i < aiGetMaterialTextureCount(mat, type); i++) {
        struct aiString path;
        enum aiTextureMapping textureMapping;
        enum aiTextureOp op;
        enum aiTextureMapMode textureMapMode[3] = {0, 0, 0};
        unsigned int uvwIndex, flags;
        float blend;
        aiGetMaterialTexture(mat, type, i, &path, &textureMapping, &uvwIndex, &blend, &op, textureMapMode, &flags);

        GLboolean skip = false;
        for (GLuint j = 0; j < model->tl_size; j++) {
            if (!strcmp(model->textures_loaded[j].directory.data, path.data)) {
                mesh->textures[index+i] = model->textures_loaded[j];
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture texture;
            texture.id = textureFromFile(path.data, model->directory);
            texture.type = (char*) calloc(sizeof(char), strlen(typeName)+1);
        strcpy(texture.type, typeName);
            texture.directory = path;
            mesh->textures[index+i] = texture;
            model->textures_loaded[model->tl_size++] = texture;
        }
    }
}

GLuint textureFromFile(const char* path, char* dir) {
    char* filename = (char*) calloc(sizeof(char), strlen(path)+strlen(dir)+2);
    strcpy(filename, dir);
    strcat(filename, "/");
    strcat(filename, path);

    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);	

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    return textureID;
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGBA);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);	

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    return textureID;
}

char* getDirectory(const char* path, char delimiter) {
    size_t last;
    for (size_t i = 0; i < strlen(path); i++) {
        if (path[i] == delimiter)
            last = i;
    }

    char* dir = (char*) calloc(sizeof(char), last+2);
    for (GLuint i = 0; i < last; i++) {
        dir[i] = path[i];
    }

    return dir;
}