#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#define CGLM_DEFINE_PRINTS
#include <cglm/cglm.h>
#define nullptr (void*)0
#include "shader.h"
#include "camera.h"
#include "model.h"

typedef struct {
	GLuint VAO;
	GLuint vertices_count;
	GLuint texture;
	vec3 position;
	GLfloat rotation_angle;
	vec3 rotation_axes;
	vec3 scale;
} Object;

typedef struct {
	Object* objects;
	GLuint objects_count;
} ObjectsList;

GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);
void key_callback(GLFWwindow*, int, int, int, int);
void mouse_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow*, double, double);
void do_movement();
void genObject(Object* object, GLuint VAO, GLuint vertices_count, GLuint texture, vec3 position, GLfloat rotation_angle, vec3 rotation_axes, vec3 scale);
void drawObject(Object object, GLuint shader);
void genObjectsList(ObjectsList* objectsList);
void appendObjectsList(ObjectsList* objectsList, Object object);
void sortObjectsList(ObjectsList* objectsList, vec3 camera_position);
void drawObjectsList(ObjectsList objectsList, GLuint shader);

const GLint WIDTH = 800, HEIGHT = 600;

Camera* camera;
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat currentFrame;


int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Still learning OpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();

	glViewport(0, 0, WIDTH, HEIGHT);

	GLuint programShader = createShaderProgram("shader.vs", "shader.frag");
	GLuint screenShader = createShaderProgram("screen.vs", "screen.frag");

	camera = newCamera((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, -1.0f}, (vec3){0.0f, 1.0f, 0.0f});

	GLfloat cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		// Right face
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left  
    };

    GLfloat planeVertices[] = {
        // Bottom face
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, // top-right
		5.0f, -0.5f, -5.0f,  1.0f, 1.0f, // top-left
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f, // bottom-left
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f, // bottom-left
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f, // bottom-right
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, // top-right
		// Top face
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, // top-left
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f, // bottom-right
		5.0f, -0.5f, -5.0f,  1.0f, 1.0f, // top-right     
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f, // bottom-right
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, // top-left
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f  // bottom-left  							
    };

	 GLfloat quadVertices[] = {   // Vertex attributes for a quad that in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

	GLuint cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	GLuint planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	GLint cubeTexture = loadTexture("textures/container2.png");
	GLint planeTexture = loadTexture("textures/stone.png");

	Object cubeObject1, cubeObject2, planeObject, testObject;
	genObject(&cubeObject1, cubeVAO, 36, cubeTexture, (vec3){1.0f, 0.01f, 0.0f}, 0.0f, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f});
	genObject(&cubeObject2, cubeVAO, 36, cubeTexture, (vec3){0.0f, 0.01f, -2.0f}, 0.0f, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f});
	genObject(&planeObject, planeVAO, 12, planeTexture, (vec3){0.0f, 0.0f, 0.0}, 0.0f, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f});

	ObjectsList objectsList;
	genObjectsList(&objectsList);
	appendObjectsList(&objectsList, cubeObject1);
	appendObjectsList(&objectsList, cubeObject2);
	appendObjectsList(&objectsList, planeObject);

	GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);  

    GLuint textureColorbuffer = generateAttachmentTexture(false, false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	GLuint renderbuffer;
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        raise("ERROR::FRAMEBUFFER", "Framebuffer is not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mat4 model, view, projection;

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window)) {
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		do_movement();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programShader);

		glm_mat4_identity(projection);
		glm_mat4_identity(view);
		cameraLookAt(camera, view);
		glm_perspective(glm_rad(camera->fov), (GLfloat)WIDTH/(GLfloat)HEIGHT, 0.1f, 100.0f, projection);
		glUniformMatrix4fv(glGetUniformLocation(programShader, "projection"), 1, GL_FALSE, *projection);
        glUniformMatrix4fv(glGetUniformLocation(programShader, "view"), 1, GL_FALSE, *view);

		drawObjectsList(objectsList, programShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(screenShader);
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &framebuffer);

	glfwTerminate();
	return 0;
}

GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil) {
    GLenum attachment_type;
    if(!depth && !stencil)
        attachment_type = GL_RGB;
    else if(depth && !stencil)
        attachment_type = GL_DEPTH_COMPONENT;
    else if(!depth && stencil)
        attachment_type = GL_STENCIL_INDEX;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(!depth || !stencil)
    	glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, WIDTH, HEIGHT, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
    else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) keys[key] = true;
	else if (action == GLFW_RELEASE) keys[key] = false;
	if (keys[GLFW_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GL_TRUE);
} 

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05f;
	cameraDirection(camera, xoffset, yoffset, sensitivity);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	GLfloat sensitivity = 2.0f;
	cameraChangeFov(camera, -yoffset, sensitivity);
}

void do_movement() {
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W]) cameraForward(camera, cameraSpeed);
	if (keys[GLFW_KEY_S]) cameraBackward(camera, cameraSpeed);
	if (keys[GLFW_KEY_A]) cameraLeft(camera, cameraSpeed);
	if (keys[GLFW_KEY_D]) cameraRight(camera, cameraSpeed);
}

void genObject(Object* object, GLuint VAO, GLuint vertices_count, GLuint texture, vec3 position, GLfloat rotation_angle, vec3 rotation_axes, vec3 scale) {
	object->VAO = VAO;
	object->vertices_count = vertices_count;
	object->texture = texture;
	glm_vec3_copy(position, object->position);
	object->rotation_angle = rotation_angle;
	glm_vec3_copy(rotation_axes, object->rotation_axes);
	glm_vec3_copy(scale, object->scale);
}

void drawObject(Object object, GLuint shader) {
	glBindVertexArray(object.VAO);
	glBindTexture(GL_TEXTURE_2D, object.texture);

	mat4 model;
	glm_mat4_identity(model);
	glm_translate(model, object.position);
	glm_rotate(model, object.rotation_angle, object.rotation_axes);
	glm_scale(model, object.scale);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, *model);
	glDrawArrays(GL_TRIANGLES, 0, object.vertices_count);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void genObjectsList(ObjectsList* objectsList) {
	objectsList->objects = (Object*) malloc(sizeof(Object));
	objectsList->objects_count = 0;
}

void appendObjectsList(ObjectsList* objectsList, Object object) {
	objectsList->objects[objectsList->objects_count] = object;
	objectsList->objects = realloc(objectsList->objects, (++objectsList->objects_count+1) * sizeof(Object));
}

void sortObjectsList(ObjectsList* objectsList, vec3 camera_position) {
	for (GLuint i = 1; i < objectsList->objects_count; i++) {
		Object current_object = objectsList->objects[i];
		for (GLint j = i-1; j >= 0; j--) {
			if (glm_vec3_distance(current_object.position, camera_position) < glm_vec3_distance(objectsList->objects[j].position, camera_position)) {
				objectsList->objects[j+1] = objectsList->objects[j];
				objectsList->objects[j] = current_object;
			} else break;
		}
	}
}

void drawObjectsList(ObjectsList objectsList, GLuint shader) {
	for (GLint i = objectsList.objects_count-1; i >= 0; i--) {
		drawObject(objectsList.objects[i], shader);
	}
}