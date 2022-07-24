#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define CGLM_DEFINE_PRINTS
#include <cglm/cglm.h>
#include "camera.h"
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
vec3 temp_vec;

Camera* newCamera(vec3 pos, vec3 front, vec3 up) {
	Camera* camera = (Camera*) malloc(sizeof(Camera));
	for (int i = 0; i < 3; i++) {
		*(camera->pos+i) = *(pos+i);
		*(camera->front+i) = *(front+i);
		*(camera->up+i) = *(up+i);
	}
	camera->pitch = 0.0f;
	camera->yaw = -90.0f;
	camera->fov = 45.0f;
	return camera;
}

void cameraForward(Camera* camera, float speed) {
	glm_vec3_muladds(camera->front, speed, camera->pos);
}

void cameraBackward(Camera* camera, float speed) {
	glm_vec3_muladds(camera->front, -speed, camera->pos);
}

void cameraRight(Camera* camera, float speed) {
	glm_vec3_cross(camera->front, camera->up, temp_vec);
	glm_vec3_normalize(temp_vec);
	glm_vec3_muladds(temp_vec, speed, camera->pos);
}

void cameraLeft(Camera* camera, float speed) {
	glm_vec3_cross(camera->front, camera->up, temp_vec);
	glm_vec3_normalize(temp_vec);
	glm_vec3_muladds(temp_vec, -speed, camera->pos);
}

void cameraLookAt(Camera* camera, mat4 view) {
	glm_vec3_add(camera->pos, camera->front, temp_vec);
	glm_lookat(camera->pos, temp_vec, camera->up, view);
}

void cameraDirection(Camera* camera, double xoffset, double yoffset, float sensitivity) {
	camera->yaw += xoffset * sensitivity;
	camera->pitch += yoffset * sensitivity;

	if (camera->pitch > 89.0f)
		camera->pitch = 89.0f;
	else if (camera->pitch < -89.0f)
		camera->pitch = -89.0f;

	glm_normalize_to((vec3){cos(glm_rad(camera->pitch)) * cos(glm_rad(camera->yaw)), sin(glm_rad(camera->pitch)), cos(glm_rad(camera->pitch)) * sin(glm_rad(camera->yaw))}, camera->front);
}

void cameraChangeFov(Camera* camera, double offset, float sensitivity) {
	camera->fov -= offset * sensitivity;
	if (camera->fov < 1.0f)
		camera->fov = 1.0f;
	if (camera->fov > 45.0f)
		camera->fov = 45.0f;
}