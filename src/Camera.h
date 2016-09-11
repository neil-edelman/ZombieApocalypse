struct Camera;
struct Widget;

struct Camera *Camera(struct Widget *target);
void Camera_(struct Camera **cameraptr);
void CameraInfo(const struct Camera *c);
struct Vector3f *CameraGetPosition(const struct Camera *camera);
void CameraUpdate(struct Camera *cam);
