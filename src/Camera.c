#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include "Vector.h"
#include "Obj.h"
#include "Widget.h"
#include "Camera.h"

static const float speed = 0.05;

struct Camera {
	struct Widget *target;
	struct Vector3f here, velocity;
};

/* public */

/** constructor */
struct Camera *Camera(struct Widget *target) {
	struct Camera *camera;

	if(!target) {
		fprintf(stderr, "Camera: no target.\n");
		return 0;
	}
	if(!(camera = malloc(sizeof(struct Camera)))) {
		perror("Camera constructor");
		Camera_(&camera);
		return 0;
	}
	camera->target = target;
	Vector3fSet(&camera->here, WidgetGetX(target));
	Vector3fZero(&camera->velocity);
	fprintf(stderr, "Camera: new, following %s, #%p.\n", WidgetToString(target), (void *)camera);

	return camera;
}

/** destructor */
void Camera_(struct Camera **cameraptr) {
	struct Camera *camera;

	if(!cameraptr || !(camera = *cameraptr)) return;
	fprintf(stderr, "~Camera: erase, #%p.\n", (void *)camera);
	free(camera);
	*cameraptr = camera = 0;
}

struct Vector3f *CameraGetPosition(const struct Camera *camera) {
	static struct Vector3f here;
	if(!camera) return &zero3f;
	here.x[0] = camera->here.x[0];
	here.x[1] = camera->here.x[1];
	here.x[2] = camera->here.x[2];
	return WidgetGetX(camera->target)/*&here*/;
}

void CameraUpdate(struct Camera *cam) {
	struct Vector3f del;
	if(!cam) return;
	Vector3fSub(&del, WidgetGetX(cam->target), &cam->here);
	Vector3fMul(&del, speed);
	Vector3fAddTo(&cam->here, &del);
	Vector3fSet(&cam->here, WidgetGetX(cam->target));
}
