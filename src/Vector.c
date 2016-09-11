#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <math.h>   /* sqrt */
#include <float.h>  /* FLTMAX */
#include "Vector.h"

struct Vector2f zero2f = { { 0, 0 } };
struct Vector3f zero3f = { { 0, 0, 0 } };

#define STRINGSIZE 32

static char toString[STRINGSIZE];

/* public */

/* not safe */
void Vector2fZero(struct Vector2f *a) { a->x[0] = a->x[1] = 0; }
void Vector2fSet(struct Vector2f *a, const struct Vector2f *b) {
	a->x[0] = b->x[0];
	a->x[1] = b->x[1];
}
void Vector2fSerial(struct Vector2f *a, const float x1, const float x2) {
	a->x[0] = x1;
	a->x[1] = x2;
}
void Vector2fAddTo(struct Vector2f *a, const struct Vector2f *b) {
	a->x[0] += b->x[0];
	a->x[1] += b->x[1];
}
void Vector2fSub(struct Vector2f *a, const struct Vector2f *b, const struct Vector2f *c) {
	a->x[0] = b->x[0] - c->x[0];
	a->x[1] = b->x[1] - c->x[1];
}
void Vector2fMul(struct Vector2f *a, const float p) {
	a->x[0] *= p;
	a->x[1] *= p;
}
void Vector2fWedge(struct Vector2f *a, const struct Vector2f *b) {
	a->x[0] = b->x[1];
	a->x[1] = b->x[0];
}
void Vector2fNormalise(struct Vector2f *a) {
	float z = 1 / sqrt(a->x[0]*a->x[0] + a->x[1]*a->x[1]);
	if(z == 0.0) {
		a->x[0] = 1;
		a->x[1] = 0;
		return;
	}
	a->x[0] *= z;
	a->x[1] *= z;
}
char *Vector2fToString(const struct Vector2f *a) {
	snprintf(toString, STRINGSIZE, "(%f, %f)", a->x[0], a->x[1]);
	return toString;
}

void Vector3fZero(struct Vector3f *a) { a->x[0] = a->x[1] = a->x[2] = 0; }
void Vector3fSet(struct Vector3f *a, const struct Vector3f *b) {
	a->x[0] = b->x[0];
	a->x[1] = b->x[1];
	a->x[2] = b->x[2];
}
void Vector3fSerial(struct Vector3f *a, const float x1, const float x2, const float x3) {
	a->x[0] = x1;
	a->x[1] = x2;
	a->x[2] = x3;
}
void Vector3fAddTo(struct Vector3f *a, const struct Vector3f *b) {
	a->x[0] += b->x[0];
	a->x[1] += b->x[1];
	a->x[2] += b->x[2];
}
void Vector3fSub(struct Vector3f *a, const struct Vector3f *b, const struct Vector3f *c) {
	a->x[0] = b->x[0] - c->x[0];
	a->x[1] = b->x[1] - c->x[1];
	a->x[2] = b->x[2] - c->x[2];
}
void Vector3fMul(struct Vector3f *a, const float p) {
	a->x[0] *= p;
	a->x[1] *= p;
	a->x[2] *= p;
}
void Vector3fWedge(struct Vector3f *a, const struct Vector3f *b, const struct Vector3f *c) {
	a->x[0] = b->x[1]*c->x[2] - b->x[2]*c->x[1];
	a->x[1] = b->x[2]*c->x[0] - b->x[0]*c->x[2];
	a->x[2] = b->x[0]*c->x[1] - b->x[1]*c->x[0];
}
void Vector3fNormalise(struct Vector3f *a) {
	float z = 1 / sqrt(a->x[0]*a->x[0] + a->x[1]*a->x[1] + a->x[2]*a->x[2]);
	if(z == 0.0) {
		a->x[0] = 1;
		a->x[1] = 0;
		a->x[2] = 0;
		return;
	}
	a->x[0] *= z;
	a->x[1] *= z;
	a->x[2] *= z;
}
char *Vector3fToString(const struct Vector3f *a) {
	if(!a) return "(null)";
	snprintf(toString, STRINGSIZE, "(%f, %f, %f)", a->x[0], a->x[1], a->x[2]);
	return toString;
}

/*
 static void VectorConvertPolar(const float theta, const float phi) {
	p->norm[0] = sin(phi) * cos(theta);
	p->norm[1] = sin(phi) * sin(theta);
	p->norm[2] = cos(phi);
	p->xunit[0] =  cos(phi) * sin(theta); * rho *
	p->xunit[1] =  cos(phi) * cos(theta); * theta *
	p->xunit[2] = -sin(phi);              * phi *
	p->p[0] = theta; p->p[1] = phi;
	p->oversin_th = (sin(theta) == 0 ? 1e11*FLT_MAX* : 1 / sin(theta));
	p->A[0] = p->A[1] = p->A[2] = 0;
	p->gain = 0;
	p->sin_th = sin(theta); p->cos_th = cos(phi);
}
*/
