struct Vector2f {
	float x[2];
};

struct Vector3f {
	float x[3];
};

struct Vector2f zero2f;
struct Vector3f zero3f;

/* intended as static */

void Vector2fZero(struct Vector2f *a);
void Vector2fSet(struct Vector2f *a, const struct Vector2f *b);
void Vector2fSerial(struct Vector2f *a, const float x1, const float x2);
void Vector2fAddTo(struct Vector2f *a, const struct Vector2f *b);
void Vector2fSub(struct Vector2f *a, const struct Vector2f *b, const struct Vector2f *c);
void Vector2fMul(struct Vector2f *a, const float p);
void Vector2fWedge(struct Vector2f *a, const struct Vector2f *b);
void Vector2fNormalise(struct Vector2f *a);
char *Vector2fToString(const struct Vector2f *a);

void Vector3fZero(struct Vector3f *a);
void Vector3fSet(struct Vector3f *a, const struct Vector3f *b);
void Vector3fSerial(struct Vector3f *a, const float x1, const float x2, const float x3);
void Vector3fAddTo(struct Vector3f *a, const struct Vector3f *b);
void Vector3fSub(struct Vector3f *a, const struct Vector3f *b, const struct Vector3f *c);
void Vector3fMul(struct Vector3f *a, const float p);
void Vector3fWedge(struct Vector3f *a, const struct Vector3f *b, const struct Vector3f *c);
void Vector3fNormalise(struct Vector3f *a);
char *Vector3fToString(const struct Vector3f *a);
