#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strdup */
#include "Text.h"
#include "Vector.h"
#include "Obj.h"

extern void (*vertex)(float, float, float);
extern void (*normal)(float, float, float);
extern void (*texel)(float, float);

/* constants */
static const int granularity = 64;

struct Obj {
	char  *fn;
	char  *skin;
	int   allocv, nv, allocf, nf, alloct, nt;
	struct Vector3f *vertices;
	struct Vector3f *normals;
	int   *faces;
	struct Vector2f *coords;
};

/* public */

/** constructor */
struct Obj *Obj(const char *fn) {
	struct Text *txt;
	char *str;
	struct Obj *obj;
	int vError = 0, fError = 0, vtError = 0, skError = 0;
	int i, j;

	if(!fn || *fn == '\0' || !(txt = Text(fn))) return 0;
	if(!(obj = malloc(sizeof(struct Obj)))) {
		perror("Obj constructor");
		Obj_(&obj);
		Text_(&txt);
		return 0;
	}
	obj->fn       = 0;
	obj->skin     = 0;
	obj->allocv   = 0;
	obj->nv       = 0;
	obj->allocf   = 0;
	obj->nf       = 0;
	obj->alloct   = 0;
	obj->nt       = 0;
	obj->vertices = 0;
	obj->normals  = 0;
	obj->faces    = 0;
	obj->coords   = 0;
	if(!(obj->fn = strdup(fn))) {
		perror(fn);
		Obj_(&obj);
		Text_(&txt);
		return 0;
	}
	while(TextGetLine(txt)) {
		/* fixme: O(n) */
		if(TextStartsWith(txt, "#")) {
			continue;
		} else if(TextStartsWith(txt, "v ")) {
			float x[3];
			int i = 0;
			TextNextToken(txt);
			while((str = TextNextToken(txt))) if(i <= 3) x[i++] = strtof(str, 0);
			if(i != 3) { vError = -1; continue; }
			/*printf("Vertex (%f %f %f)\n", x[0], x[1], x[2]);*/
			if(obj->nv + 1 > obj->allocv) {
				int newallocv = obj->allocv + granularity;
				struct Vector3f *newver = realloc(obj->vertices, newallocv*sizeof(struct Vector3f));
				if(!newver) {
					perror(fn);
					Obj_(&obj);
					Text_(&txt);
					return 0;
				}
				obj->allocv   = newallocv;
				obj->vertices = newver;
				/* spam printf("Obj: allocated %d vertices.\n", obj->allocv); */
			}
			Vector3fSerial(&obj->vertices[obj->nv], x[0], x[1], x[2]);
			obj->nv++;
		} else if(TextStartsWith(txt, "f ")) {
			int s[3];
			int i = 0;
			TextNextToken(txt);
			while((str = TextNextToken(txt))) if(i <= 3) s[i++] = strtol(str, 0, 0);
			if(i != 3) { fError = -1; continue; }
			/*printf("Face (%d %d %d)\n", s[0], s[1], s[2]);*/
			if(obj->nf + 1 > obj->allocf) {
				int newallocf = obj->allocf + granularity;
				int *newfac = realloc(obj->faces, newallocf*3 * sizeof(int));
				if(!newfac) {
					perror(fn);
					Obj_(&obj);
					Text_(&txt);
					return 0;
				}
				obj->allocf = newallocf;
				obj->faces  = newfac;
				/* spam printf("Obj: allocated %d faces.\n", obj->allocf);*/
			}
			obj->faces[obj->nf*3 + 0] = s[0] - 1;
			obj->faces[obj->nf*3 + 1] = s[1] - 1;
			obj->faces[obj->nf*3 + 2] = s[2] - 1;
			obj->nf++;
		} else if(TextStartsWith(txt, "vt ")) {
			float t[3];
			int i = 0;
			TextNextToken(txt);
			while((str = TextNextToken(txt))) if(i <= 2) t[i++] = strtof(str, 0);
			if(i != 2) { vtError = -1; continue; }
			if(obj->nt + 1 > obj->alloct) {
				int newalloct = obj->alloct + granularity;
				struct Vector2f *newtex = realloc(obj->coords, newalloct*sizeof(struct Vector2f));
				if(!newtex) {
					perror(fn);
					Obj_(&obj);
					Text_(&txt);
					return 0;
				}
				obj->alloct = newalloct;
				obj->coords = newtex;
				/* spam printf("Obj: allocated %d vertices.\n", obj->alloct); */
			}
			Vector2fSerial(&obj->coords[obj->nt], t[0], t[1]);
			obj->nt++;
		} else if(TextStartsWith(txt, "skin ")) {
			if(obj->skin) { skError = -1; continue; }
			TextNextToken(txt);
			if(!(str = TextEndOfLine(txt))) { skError = -1; continue; }
			obj->skin = strdup(str);
		}
	}
	Text_(&txt);
	/* audit obj */
	if(vError)  fprintf(stderr, "Obj: <%s>, warning, non-3d vertices ignored.\n", fn);
	if(fError)  fprintf(stderr, "Obj: <%s>, warning, non-trianglar face ignored.\n", fn);
	if(vtError) fprintf(stderr, "Obj: <%s>, warning, non-2d texture coordinates ignored.\n", fn);
	if(skError) fprintf(stderr, "Obj: <%s>, warning, skin is idempotent; other skins ignored.\n", fn);
	/* check faces are in range of vertices */
	for(i = 0; i < obj->nf; i++) {
		if(obj->faces[i*3 + 0] == obj->faces[i*3 + 1] ||
		   obj->faces[i*3 + 0] == obj->faces[i*3 + 2] ||
		   obj->faces[i*3 + 1] == obj->faces[i*3 + 2]) {
			fprintf(stderr, "Obj: <%s>, error, contains faces that duplicate vertices.\n", fn);
		}
		for(j = 0; j < 3; j++) {
			if(obj->faces[i*3+j] < 0 || obj->faces[i*3+j] >= obj->nv) {
				fprintf(stderr, "Obj: <%s>, error, face vertex index is out of range.\n", fn);
			}
		}
	}
	/* num tex coords == num vertices, or it makes no sense and we should not
	 texture it! */
	if(obj->nv != obj->nt && obj->coords) {
		fprintf(stderr, "Obj: <%s>, warning, skin does not fit vertices.\n", fn);
		free(obj->coords);
		obj->nt = 0;
	}
	/* fixme: use heuristic like cylidrical or ellispsodal */
	/*if(!obj->coords && obj->skin) {
		fprintf(stderr, "Obj: <%s>, warning, skin does not have texture coordinates.\n", fn);
		free(obj->skin);
		obj->skin = 0;
	} fixed? */
	/* fixme: make sure the object if a manifold, check for 2 points the same, etc */
	/* allocate normals */
	if(!(obj->normals = malloc(sizeof(struct Vector3f)*obj->nv))) {
		perror(fn);
		Obj_(&obj);
		Text_(&txt);
		return 0;
	}
	/* calculate face normals; fixme: we assume that the models are not going
	 to cause div by zero; (we have a lot of confidence!) */
	for(i = 0; i < obj->nf; i++) {
		int f1 = obj->faces[i*3 + 0];
		int f2 = obj->faces[i*3 + 1];
		int f3 = obj->faces[i*3 + 2];
		struct Vector3f v1 = obj->vertices[f1];
		struct Vector3f v2 = obj->vertices[f2];
		struct Vector3f v3 = obj->vertices[f3];
		struct Vector3f v12, v13, fn;

		Vector3fSub(&v12, &v1, &v2);
		Vector3fSub(&v13, &v1, &v3);
		Vector3fWedge(&fn, &v12, &v13);
		Vector3fAddTo(&obj->normals[f1], &fn);
		Vector3fAddTo(&obj->normals[f2], &fn);
		Vector3fAddTo(&obj->normals[f3], &fn);
	}
	/* normalise vertex normals */
	for(i = 0; i < obj->nv; i++) Vector3fNormalise(&obj->normals[i]);
	/*free(facen);*/
	fprintf(stderr, "Obj: new %s v%d f%d, skin %s, #%p.\n", fn, obj->nv, obj->nf, obj->skin ? obj->skin : "none", (void *)obj);

	return obj;
}

/** destructor */
void Obj_(struct Obj **objptr) {
	struct Obj *obj;

	if(!objptr || !(obj = *objptr)) return;
	fprintf(stderr, "~Obj: erase, #%p.\n", (void *)obj);
	if(obj->coords) free(obj->coords);
	if(obj->faces) free(obj->faces);
	if(obj->normals) free(obj->normals);
	if(obj->vertices) free(obj->vertices);
	if(obj->skin) free(obj->skin);
	if(obj->fn) free(obj->fn);
	free(obj);
	*objptr = obj = 0;
}

void ObjInfo(const struct Obj *o) {
	int i, j;
	if(!o) {
		printf("Obj <null>.\n");
		return;
	}
	printf("Obj %s: v %d, f %d\\\n", o->fn, o->nv, o->nf);
	for(i = 0; i < o->nf; i++) {
		printf(" f%d:\n", i+1);
		for(j = i*3; j < i*3 + 3; j++) {
			printf("  v%d%sn%s\n", o->faces[j]+1, Vector3fToString(&o->vertices[o->faces[j]]), Vector3fToString(&o->normals[o->faces[j]]));
		}
	}
}

char *ObjGetSkin(const struct Obj *o) {
	if(!o) return 0;
	return o->skin;
}

int ObjGetNumCoords(const struct Obj *o) {
	if(!o) return 0;
	return o->nt;
}

/** this is called inside glBegin(GL_TRIANGLES)
 fixme: will be (_STRIP)) */
void ObjDraw(const struct Obj *o) {
	int f, v1, v2, v3;
	if(!o) return;
	for(f = 0; f < o->nf; f++) {
		v1 = o->faces[f*3 + 0];
		v2 = o->faces[f*3 + 1];
		v3 = o->faces[f*3 + 2];
		if(o->coords) texel(o->coords[v1].x[0], o->coords[v1].x[1]);
		normal(o->normals[v1].x[0], o->normals[v1].x[1], o->normals[v1].x[2]);
		vertex(o->vertices[v1].x[0], o->vertices[v1].x[1], o->vertices[v1].x[2]);
		if(o->coords) texel(o->coords[v2].x[0], o->coords[v2].x[1]);
		normal(o->normals[v2].x[0], o->normals[v2].x[1], o->normals[v2].x[2]);
		vertex(o->vertices[v2].x[0], o->vertices[v2].x[1], o->vertices[v2].x[2]);
		if(o->coords) texel(o->coords[v3].x[0], o->coords[v3].x[1]);
		normal(o->normals[v3].x[0], o->normals[v3].x[1], o->normals[v3].x[2]);
		vertex(o->vertices[v3].x[0], o->vertices[v3].x[1], o->vertices[v3].x[2]);
	}
}

/* triangle */
