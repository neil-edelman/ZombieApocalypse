#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include "Vector.h"
#include "Widget.h"
#include "Input.h"

struct Input {
	enum Toggle left;
	enum Toggle up;
	enum Toggle right;
	enum Toggle down;
	enum Toggle go;
	struct Vector3f position;
	enum Toggle status;
};

/* public */

/** constructor */
struct Input *Input(void) {
	struct Input *input;

	if(!(input = malloc(sizeof(struct Input)))) {
		perror("Input constructor");
		Input_(&input);
		return 0;
	}
	/* fixme: would it be better to check if they were down? */
	input->left  = t_off;
	input->up    = t_off;
	input->right = t_off;
	input->down  = t_off;
	input->go    = t_off;
	Vector3fZero(&input->position);
	input->status= t_off;
	fprintf(stderr, "Input: new, #%p.\n", (void *)input);

	return input;
}

/** destructor */
void Input_(struct Input **inputptr) {
	struct Input *input;

	if(!inputptr || !(input = *inputptr)) return;
	fprintf(stderr, "~Input: erase, #%p.\n", (void *)input);
	free(input);
	*inputptr = input = 0;
}

void InputKey(struct Input *i, char k, enum Toggle stat) {
	if(!i) return;
	switch(k) {
		case k_left:
			i->left  = stat;
			break;
		case k_up:
			i->up    = stat;
			break;
		case k_right:
			i->right = stat;
			break;
		case k_down:
			i->down  = stat;
			break;
		case 13:
			printf("Input::key: escape pressed.\n");
			exit(EXIT_SUCCESS);
		case 'a':
			i->status = stat;
			break;
	}
	printf("Key %d turned %s (%d).\n", k, stat ? "on" : "off", k_f1);
}

void InputSetPosition(struct Input *i, const struct Vector3f *p) {
	if(!i || !p) return;
	Vector3fSet(&i->position, p);
	i->go = t_on;
}

struct Vector3f *InputGetPosition(struct Input *i) {
	if(!i) return 0;
	return &i->position;
}

void OpenPrintCam();

void InputUpdate(struct Input *i, struct Widget *w) {
	struct Vector3f *x, d;
	if(!i || !w) return;
	x = WidgetGetX(w);
	if(i->left) { x->x[0] -= 0.05; i->go = t_off; }
	if(i->up)   { x->x[2] -= 0.05; i->go = t_off; }
	if(i->right){ x->x[0] += 0.05; i->go = t_off; }
	if(i->down) { x->x[2] += 0.05; i->go = t_off; }
	if(i->go) {
		Vector3fSub(&d, &i->position, x);
		/*Vector3fNormalise(&d);*/
		Vector3fMul(&d, 0.05);
		Vector3fAddTo(x, &d);
		/* test if we're there
		if(Vector3f...()) i->go = t_off; */
	}
	if(i->status) { OpenPrintCam(); i->status = t_off; }
}
