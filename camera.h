#ifndef _CAMERA_H
#define _CAMERA_H
#include <cglm/cglm.h>

typedef struct {
	vec3 pos;
	vec3 front;
	vec3 up;
	float pitch;
	float yaw;
	float fov;
} Camera;

Camera* newCamera(vec3 pos, vec3 front, vec3 up);
void cameraForward(Camera* camera, float speed);
void cameraBackward(Camera* camera, float speed);
void cameraRight(Camera* camera, float speed);
void cameraLeft(Camera* camera, float speed);
void cameraLookAt(Camera* camera, mat4 view);
void cameraDirection(Camera* camera, double xoffset, double yoffset, float sensitivity);
void cameraChangeFov(Camera* camera, double offset, float sensitivity);

#endif