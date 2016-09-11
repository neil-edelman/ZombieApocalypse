#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <stdarg.h> /* vsnprintf (in OpenPrint()) */
#include <string.h> /* strstr (in loading fns,) strrchr (in guess()) */
#include <math.h>   /* tan (in resize()) */
#ifdef GL
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h> /* OpenGL */
#include <GLUT/glut.h> /* GLUT */
#endif
#include <unistd.h>   /* (in guess()) chdir (POSIX, not ANSI) */
#include <dirent.h>   /* opendir readdir closedir (in loading fns) */
#include <sys/stat.h> /* fstat */
#include "Input.h"
#include "Map.h"
#include "Bitmap.h"
#include "Vector.h"
#include "Camera.h"
#include "Obj.h"
#include "Widget.h"
#include "Buffer.h"
#include "Open.h"

struct Open {
	struct Input *input;
	struct Map *map;
	struct Camera *camera;
	double xFrustum, yFrustum;
	int        frame;
	float      time;
	int        fps;
	struct Buffer *textures;
	struct Buffer *models;
	struct Widget *widgets;
	/*char       con[LINE];*/
};

struct Texture {
	char key[16]; /* only the first 15 characters will be recognized! */
	GLuint name;
};

struct Model {
	char key[16];
	GLuint list;
};

int main(int argc, char **argv);
int load(void);
void unload(void);
void update(int);
void display(void);
void draw(void);
void resize(int width, int height);
void key_down(unsigned char k, int x, int y);
void key_up(unsigned char k, int x, int y);
void key_down_special(int k, int x, int y);
void key_up_special(int k, int x, int y);
enum Keys opengl_to_input(int k);
void mouse(int button, int state, int x, int y);
void mouse_walk(const int x, const int y);
void mouse_target(const int x, const int y);
void selection(const int x, const int y);
void guess(const int argc, char **argv);

int tex_comp(const struct Texture *a, const struct Texture *b);
int mod_comp(const struct Model *a, const struct Model *b);
void tex_print(const struct Texture *a);
void mod_print(const struct Model *a);
	
/* global! */
static struct Open *open = 0;
/* fixme: how do I declare these as const? */
void (*vertex)(float, float, float) = &glVertex3f;
void (*normal)(float, float, float) = &glNormal3f;
void (*texel)(float, float) = &glTexCoord2f;

/* constants */

const static char dirsep = '/';
const static char *dircurrent = ".";
const static char extension = '.';
const static char *ext_tex = "bmp";
const static char *ext_mod = "obj";
const static char *ext_map = "map";
const static char *data = "data";

const static float black_of_space[] = { 0, 0, 0, 0 };
const static float global_illum[] = { 0, 0.1, 0, 1 };
const static float sun_pos[] = { -0.5, 10, 0.1, 0 };
const static float sun_colour[] = { 1, 1, 1, 1 };
const static float exp_pos[] = { 2, 2, 2, 1 };
const static float exp_colour[] = { 1, 0, 0, 1 };
const static float one[] = { 1, 1, 1, 1 };

const static int    map_size = 8;
const static double camera_angle = 80;
const static double camera_fov = 90;
const static double camera_alt = 4;
const static double camera_near = 1;
const static double camera_far = 6;
const static float  mouse_size = 6;

const static int time_step = 25;

int main(int argc, char **argv) {
	/* guess the working directory of the programme */
	guess(argc, argv);
	/* negotiate with library */
	glutInit(&argc, argv);
	if(!Open(640, 480, "Important Work")) return EXIT_FAILURE;
	/* atexit because the loop never returns */
	if(atexit(&Open_)) perror("~Open");
	glutMainLoop();

	return EXIT_SUCCESS;
}

struct Open *Open(const int width, const int height, const char *title) {
	struct Model *model;
	struct Widget *player, *rock = 0;

	if(open || width <= 0 || height <= 0 || !title) {
		fprintf(stderr, "Open: error initialising.\n");
		return 0;
	}
	if(!(open = malloc(sizeof(struct Open)))) {
		perror("Open constructor");
		Open_();
		return 0;
	}
	open->input   = 0;
	open->map     = 0;
	open->camera  = 0;
	open->time    = glutGet(GLUT_ELAPSED_TIME);
	open->xFrustum= open->yFrustum = 0;
	open->frame   = 0;
	open->fps     = 0;
	open->textures= 0;
	open->models  = 0;
	open->widgets = 0;
	/*open->con[0]  = 'A';
	open->con[1]  = 'r';
	open->con[2]  = 'r';
	open->con[3]  = '\0';*/
	fprintf(stderr, "Open: new, #%p.\n", (void *)open);

	if(!(open->textures = Buffer(sizeof(struct Texture), (int (*)(const void *, const void *))tex_comp, (void (*)(const void *))tex_print)) ||
	   !(open->models = Buffer(sizeof(struct Model), (int (*)(const void *, const void *))mod_comp, (void (*)(const void *))mod_print))) {
		Open_();
		return 0;
	}
	/* input commands */
	if(!(open->input = Input())) {
		Open_();
		return 0;
	}

	/* initial conditions; RGB[A] is implied; create size just a suggestion */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH/* | GLUT_STENCIL*/);
	glutInitWindowSize(width, height);
	glutCreateWindow(title);
	printf("Open: opened GLUT window; %s, %s, %s.\n", glGetString(GL_VENDOR), glGetString(GL_VERSION), glGetString(GL_RENDERER));
	glutReshapeWindow(width, height);
	/*if(glutGet(GLUT_WINDOW_STENCIL_SIZE) < 2) {
		fprintf(stderr, "Open: window was created with less then 2 bits stencil-buffer.\n");
		Open_();
		return 0;
	}*/

	/* load all the data from files */
	if(!load()) {
		Open_();
		return 0;
	}
	/* invoke an instance */
	if(!(model = BufferFind(open->models, "monkey.obj"))) {
		fprintf(stderr, "Open: didn't find monkey.\n");
		Open_();
		return 0;
	}
	if(!(player = Widget(t_player, model->list, open->input))) {
		Open_();
		return 0;
	}
	WidgetLink(player, &open->widgets);
	if((rock = Widget(t_rock, model->list))) {
		struct Vector3f *v;
		v = WidgetGetX(rock);
		v->x[1] = 1;
		WidgetLink(rock, &open->widgets);
	}
	/* set a couple of zombies */
	if(!(model = BufferFind(open->models, "Zombie.obj"))) {
		fprintf(stderr, "Open: didn't find zombie.\n");
	} else {
		struct Vector3f *v;
		int i;
		for(i = 0; i < 8; i++) {
			if(!(rock = Widget(t_rock, model->list))) continue;
			v = WidgetGetX(rock);
			v->x[0] = (float)rand() / RAND_MAX * MapGetSize(open->map);
			v->x[2] = (float)rand() / RAND_MAX * MapGetSize(open->map);
			WidgetLink(rock, &open->widgets);
		}
	}
	/* and a rock */
	if(!(model = BufferFind(open->models, "box.obj"))) {
		fprintf(stderr, "Open: didn't find box.\n");
	} else if((rock = Widget(t_rock, model->list))) {
		/*struct Vector3f *v = WidgetGetX(rock);
		v->x[0] = (float)rand() / RAND_MAX * MapGetSize(open->map);
		v->x[2] = (float)rand() / RAND_MAX * MapGetSize(open->map);*/
		WidgetLink(rock, &open->widgets);
	}
	WidgetPrint(open->widgets);
	/* attach a camera */
	if(!(open->camera = Camera(player))) {
		Open_();
		return 0;
	}

	/* cull; tex; z-buff hack; clear colour */
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glClearColor(black_of_space[0], black_of_space[1], black_of_space[2], black_of_space[3]);
	/*glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);*/
	/*hurts performance->glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
	/*hurts performane->glEnable(GL_LINE_SMOOTH);*/

	/* lighting; colour tracking (with glColor, faster;) interpolate Gourad */
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, one);
	/* glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
	 glMaterialfv(GL_FRONT, GL_SHININESS, 0);*/
	glShadeModel(GL_SMOOTH);
	/* glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1); slow, don't need */
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_illum);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_colour);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, exp_colour);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	/* interesting effects, GL_EYE_LINEAR, GL_OBJECT_LINEAR, GL_SPHERE_MAP */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	/* set callbacks */
	glutDisplayFunc(&display);
	glutReshapeFunc(&resize);
	glutKeyboardFunc(&key_down);
	glutKeyboardUpFunc(&key_up);
	glutSpecialFunc(&key_down_special);
	glutSpecialUpFunc(&key_up_special);
	glutMouseFunc(&mouse);
	/* glutIdleFunc(0); disable */
	glutTimerFunc(25, update, 0);

	/* lots of room for optimizing!
	glBindBuffer();
	glEnable(GL_VERTEX_ARRAY);
	glBufferData(GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER, STATIC;STREAM;DYNAMIC, DRAW;COPY;READ(DRAW?));
	glMapBuffer(GL_WRITE_ONLY|gl_reaad_only);
	glDrawArrays(GL_TRIANGLE_STRIP, first, count);
	glUnmapBuffer();*/

	/* get the initialisation error code; fixme: do sth about it? */
	fprintf(stderr, "Open: OpenGL initilisation status, \"%s.\"\n", gluErrorString(glGetError()));

	return open;
}

void Open_(void) {
	if(!open) return;
	fprintf(stderr, "~Open: OpenGL final status \"%s;\" deleting #%p.\n", gluErrorString(glGetError()), (void *)open);
	Input_(&open->input);
	Camera_(&open->camera);
	Widget_(&open->widgets);
	Map_(&open->map);
	unload();
	free(open);
	open = 0;
}

void OpenPrint(const char *fmt, ...) {
	/*GLUT_BITMAP_8_BY_13
	 GLUT_BITMAP_9_BY_15
	 GLUT_BITMAP_TIMES_ROMAN_10
	 GLUT_BITMAP_TIMES_ROMAN_24
	 GLUT_BITMAP_HELVETICA_10
	 GLUT_BITMAP_HELVETICA_12
	 GLUT_BITMAP_HELVETICA_18 */
	/*va_list ap;*/

	if(!fmt) return;
	/* print the chars into the buffer */
	/*va_start(ap, fmt);
	vsnprintf(open->con, LINE, fmt, ap);
	va_end(ap);
	printf("%s\n", open->con);*/
}

void OpenPrintCam(void) {
	struct Vector3f *a = CameraGetPosition(open->camera);
	printf("Cam: %s\n", Vector3fToString(a));
}

/* private */

int load(void) {
	struct dirent *de;
	struct stat   st;
	DIR           *dir;
	struct Buffer *buf;
	struct Bitmap *bmp;
	char          *ext;

	if(!(dir = opendir(dircurrent))) { perror(dircurrent); return 0; }
	/* max size texture bmp */
	if(!(buf = Buffer(sizeof(char), 0, 0))) return 0;
	while((de = readdir(dir))) {
		if(!de->d_name || 0) continue;
		if(stat(de->d_name, &st)) { perror(de->d_name); continue; }
		printf(" -- file <%s>\n", de->d_name);
		if(S_ISDIR(st.st_mode)) continue;
		if(!(ext = strrchr(de->d_name, extension))) continue;
		ext++;
		/* pick which one; fixme: O(n) */
		if(!strcmp(ext, ext_tex)) {
			struct Texture *tex;
			if(!(tex = BufferAdd(open->textures))) continue;
			strncpy(tex->key, de->d_name, sizeof(tex->key)/sizeof(char));
			tex->key[sizeof(tex->key)/sizeof(char)-1] = '\0';
			tex->name = 0;
			/*printf("BufferAdd textures = tex_ptr %p\n", (void *)tex_ptr);*/
			bmp = Bitmap(de->d_name, buf);
			/*printf("Bitmap %s ", de->d_name); BufferPrint(buf);*/
			glGenTextures(1, &tex->name);
			glBindTexture(GL_TEXTURE_2D, tex->name);
			/* don't do any special settings, comments are defauts already */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT/*GL_CLAMP_TO_EDGE*/);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT/*GL_CLAMP_TO_EDGE*/);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			/* 2=mipmap, 3=colours(or GL_RGB, GL_RGB8 is faster?), 6=border */
			glTexImage2D(GL_TEXTURE_2D, 0, 3, BitmapGetWidth(bmp), BitmapGetHeight(bmp),
						 0, GL_BGR, GL_UNSIGNED_BYTE, BitmapGetData(bmp));
			/*glPixelStorei(GL_UNPACK_ALIGNMENT, 1);*/
			/*glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);*/
			/*not part of the texture:*/
			/*glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);*/
			Bitmap_(&bmp);
			fprintf(stderr, "Open::load: <%s> assigned texture %u.\n", tex->key, tex->name);
		} else if(!strcmp(ext, ext_mod)) {
			struct Obj *obj;
			struct Model *mod;
			struct Texture *tex = 0;
			char *skin;
			if(!(mod = BufferAdd(open->models))) continue;
			strncpy(mod->key, de->d_name, sizeof(mod->key)/sizeof(char));
			mod->key[sizeof(mod->key)/sizeof(char)-1] = '\0';
			mod->list = 0;
			if(!(obj = Obj(de->d_name))) { BufferSub(open->models); continue; }
			if(!(mod->list = glGenLists(1))) {
				fprintf(stderr, "Open::load: couldn't get <%s> to a display list.\n", de->d_name);
				Obj_(&obj);
				BufferSub(open->models);
				continue;
			}
			glNewList(mod->list, GL_COMPILE);
			if((skin = ObjGetSkin(obj)) && (tex = BufferFind(open->textures, skin))) {
				glBindTexture(GL_TEXTURE_2D, tex->name);
				if(!ObjGetNumCoords(obj)) {
					glEnable(GL_TEXTURE_GEN_S);
					glEnable(GL_TEXTURE_GEN_T);
				}
			} else {
				glColor3f(0.5,0.5,0.5);
			}
			glBegin(GL_TRIANGLES);
			ObjDraw(obj);
			glEnd();
			if(tex) {
				glBindTexture(GL_TEXTURE_2D, 0);
				if(!ObjGetNumCoords(obj)) {
					glDisable(GL_TEXTURE_GEN_S);
					glDisable(GL_TEXTURE_GEN_T);
				}				
			} else {
				glColor3f(1, 1, 1);
			}
			glEndList();
			Obj_(&obj);
			fprintf(stderr, "Open::load: <%s> stored as display list %u.\n", mod->key, mod->list);
		} else if(!strcmp(ext, ext_map)) {
			if(open->map) {
				fprintf(stderr, "Open::load: <%s> ignored, idempotent.\n", de->d_name);
			}
			open->map = Map(de->d_name);
		}
	}
	Buffer_(&buf);
	closedir(dir);
	/* sort the data so we can Buffer::find */
	BufferSort(open->textures);
	BufferSort(open->models);
	fprintf(stderr, "Open::load: buffers, sorted:\n");
	BufferPrint(open->textures);
	BufferPrint(open->models);
	return -1;
}

void unload(void) {
	struct Texture *tex;
	struct Model   *mod;

	/* fixme: BufferPop() */
	while((tex = BufferGetNext(open->textures))) {
		fprintf(stderr, "Open::unload: deleting texture <%s> (name %u.)\n", tex->key, tex->name);
		glDeleteTextures(1, &tex->name);
	}
	Buffer_(&open->textures);
	while((mod = BufferGetNext(open->models))) {
		fprintf(stderr, "Open::unload: deleting <%s> (display list %u.)\n", mod->key, mod->list);
		glDeleteLists(mod->list, 1);
	}
	Buffer_(&open->models);
}

void update(int value) {
	const float time_s = time_step / 1000.0;

	CameraUpdate(open->camera);
	glutPostRedisplay();
	glutTimerFunc(time_step, update, 0);
	open->time += time_s;
	WidgetUpdate(open->widgets, time_s);
}

void display(void) {
	GLenum err;
	int time = -glutGet(GLUT_ELAPSED_TIME);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/);
	draw();
	glutSwapBuffers();
	/*glFlush(); double buffered, not needed */
	time += glutGet(GLUT_ELAPSED_TIME);
	open->frame++;
	open->fps = open->frame / open->time;
	if((err = glGetError()) != GL_NO_ERROR) fprintf(stderr, "Open::display: OpenGL error \"%s.\"\n", gluErrorString(err));
}

void draw(void) {
	struct Widget *w;
	struct Texture *arr;
	struct Vector3f *eye = CameraGetPosition(open->camera), *v;
	int name = 100;

	int x, z, size;
	struct Vector3f *n;
	float y;
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(0, 0, -camera_alt);
	glRotatef(camera_angle, 1, 0, 0);
	glTranslatef(-eye->x[0], -eye->x[1], -eye->x[2]);
	glLightfv(GL_LIGHT0, GL_POSITION, sun_pos);
	glLightfv(GL_LIGHT1, GL_POSITION, exp_pos);

	/* so we have a name to load */
	glPushName(0);

	/* terrian */
	if(open->map) {
		glLoadName(1);
		if((arr = BufferFind(open->textures, "arr.bmp"))) glBindTexture(GL_TEXTURE_2D, arr->name);
		glColor3f(1, 1, 1); /* really? want no colour */
		glBegin(GL_TRIANGLE_STRIP);
		MapDraw(open->map, eye);
		glEnd();
		/*if(arr)*/ glBindTexture(GL_TEXTURE_2D, 0);	
		size = MapGetSize(open->map);
		glLineWidth(0.1);
		glColor3f(1,0,0);
		glBegin(GL_LINES);
		for(z = 0; z < size; z++) {
			for(x = 0; x < size; x++) {
				if(!(n = MapGetNormal(open->map, x, z))) continue;
				y = MapGetAltitude(open->map, x, z);
				vertex(x, y, z);
				vertex(x+n->x[0], y+n->x[1], z+n->x[2]);
			}
		}
		glEnd();
		glColor3f(0,0,1);
		glLineWidth(1.5);
		glBegin(GL_LINE_STRIP);
		MapDraw(open->map, eye);
		glEnd();
		glColor3f(1,1,1);
	}

	for(w = open->widgets; w; w = WidgetGetNext(w)) {
		v = WidgetGetX(w);
		glLoadName(name++);
		glPushMatrix();
		glTranslatef(v->x[0], v->x[1], v->x[2]);
		glCallList(WidgetGetModel(w));
		glPopMatrix();
	}

	glPointSize(20);
	glColor3f(0, 1, 0);
	glBegin(GL_POINTS);
	vertex(0, 0, 0);
	vertex(eye->x[0], eye->x[1], eye->x[2]);
	glEnd();

	/* clear the name */
	glPopName();
}

void resize(int width, int height) {
	double xMax, yMax, aspect;

	if(width <= 0 || height <= 0) return;

	aspect = (double)width / height;
	yMax   = camera_near * tan(camera_fov * M_PI / 360.0);
	xMax   = yMax * aspect;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xMax, xMax, -yMax, yMax, camera_near, camera_far);

	open->xFrustum = xMax;
	open->yFrustum = yMax;
}

void key_down(unsigned char k, int x, int y) {
	if(k >= 128 || k == k_unknown) {
		printf("Open::key_down: key %d unknown, ignored.\n", k);
		return;
	}
	InputKey(open->input, k, t_on);
}

void key_up(unsigned char k, int x, int y) {
	if(k >= 128 || k == k_unknown) return;
	InputKey(open->input, k, t_off);
}

void key_down_special(int k, int x, int y) {
	enum Keys key = opengl_to_input(k);
	if(key == k_unknown) {
		printf("Open::key_down_special: unknown key; ignoring.\n");
		return;
	}
	InputKey(open->input, key, t_on);
}

void key_up_special(int k, int x, int y) {
	enum Keys key = opengl_to_input(k);
	if(key == k_unknown) return;
	InputKey(open->input, key, t_off);
}

enum Keys opengl_to_input(int k) {
	switch(k) {
		case GLUT_KEY_F1:     return k_f1;
		case GLUT_KEY_F2:     return k_f2;
		case GLUT_KEY_F3:     return k_f3;
		case GLUT_KEY_F4:     return k_f4;
		case GLUT_KEY_F5:     return k_f5;
		case GLUT_KEY_F6:     return k_f6;
		case GLUT_KEY_F7:     return k_f7;
		case GLUT_KEY_F8:     return k_f8;
		case GLUT_KEY_F9:     return k_f9;
		case GLUT_KEY_F10:    return k_f10;
		case GLUT_KEY_F11:    return k_f11;
		case GLUT_KEY_F12:    return k_f12;
		case GLUT_KEY_LEFT:   return k_left;
		case GLUT_KEY_UP:     return k_up;
		case GLUT_KEY_RIGHT:  return k_right;
		case GLUT_KEY_DOWN:   return k_down;
		case GLUT_KEY_PAGE_UP:return k_pgup;
		case GLUT_KEY_PAGE_DOWN: return k_pgdn;
		case GLUT_KEY_HOME:   return k_home;
		case GLUT_KEY_END:    return k_end;
		case GLUT_KEY_INSERT: return k_ins;
		default: return k_unknown;
	}
}

void mouse(int button, int state, int x, int y) {
	if(state != GLUT_DOWN) return;
	if(button == GLUT_LEFT_BUTTON)       mouse_walk(x, y);
	else if(button == GLUT_RIGHT_BUTTON) mouse_target(x, y);
}

void mouse_walk(const int x, const int y) {
	struct Vector3f pos;
	GLdouble model[16], project[16];
	GLfloat z;
	GLint view[4];
	GLdouble x_d, y_d, z_d;
	int gl_y;

	/* get things; returns [x, y, width, height] in view; want cartesian */
	glGetDoublev(GL_PROJECTION_MATRIX, project);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, view);
	gl_y = view[3] - y;
	/* x, y, width, height, want, don't do anything with the float, return */
	glReadPixels(x, gl_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	if(!gluUnProject(x, gl_y, z, model, project, view, &x_d, &y_d, &z_d)) {
		printf("Open::mouse_walk: unproject failed.\n");
		return;
	}
	Vector3fSerial(&pos, x_d, y_d, z_d);
	InputSetPosition(open->input, &pos);
	printf("Open::mouse_walk: (%d, %d, %f)->%s.\n", x, gl_y, z, Vector3fToString(&pos));
}

void mouse_target(const int x, const int y) {
	GLuint buf[512];
	GLint view[4];
	unsigned int i, hits;
	const size_t bufsize = sizeof(buf)/sizeof(GLuint);

	glSelectBuffer(bufsize, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	
	/* pick just in the space around the cursor */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, view); /* fixme: slow! x, y, width, height */
	gluPickMatrix(x, view[3] - y, mouse_size, mouse_size, view);
	glFrustum(-open->xFrustum, open->xFrustum, -open->yFrustum, open->yFrustum, camera_near, camera_far);
	
	draw();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	hits = glRenderMode(GL_RENDER);
	if(hits >= (bufsize>>2)) hits = (bufsize>>2) - 1;
	printf("Open::mouse_target: (%d, %d)->hits %d:\n", x, y, hits);
	for(i = 0; i < hits; i++) {
		printf(" number %u; label %d\n", buf[(i<<2)+0], buf[(i<<2)+3]);
	}
}

/*void console(void) {
	char *a;
	glColor3f(0, 1, 0);
	glRasterPos2f(1, 1);
	for(a = open->con; *a != 0; a++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *a);
	}
}*/

/*void shadow() {
	
}*/

/** very scetch, attempts to locate where the programme is being run and cd to
 the data */
void guess(const int argc, char **argv) {
	char *r;
	if(argc <= 0 || !argv) {
		fprintf(stderr, "Argument count is %d; wierd, probably will crash soon.\n", argc);
		return;
	}
	if(!(r = strrchr(argv[0], dirsep))) { /* scetch x 1000000 */
		fprintf(stderr, "No path information found in arguments; leaving working directory alone.\n");
		return;
	}
	fprintf(stderr, "Guessing directory <%s%c%s>.\n", argv[0], dirsep, data);
	*r = '\0';
	if(chdir(argv[0])) perror(argv[0]);
	*r = dirsep;
	if(chdir(data)) {
		perror(data);
		fprintf(stderr, "This is probably not the data directory; what the heck, it could be.\n");
	}
}

int tex_comp(const struct Texture *a, const struct Texture *b) {
	return strcmp(a->key, b->key);
}

int mod_comp(const struct Model *a, const struct Model *b) {
	return strcmp(a->key, b->key);
}

void tex_print(const struct Texture *a) {
	if(!a) return;
	printf("%s(%u)", a->key, a->name);
}

void mod_print(const struct Model *a) {
	if(!a) return;
	printf("%s(%u)", a->key, a->list);
}
