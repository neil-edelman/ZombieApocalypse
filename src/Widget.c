#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <stdarg.h>
#include "Vector.h"
#include "Input.h"
#include "Widget.h"

struct Rock {
	struct Vector3f v;
};

struct Player {
	struct Input *input;
	struct Vector3f v;
	int hp;
};

struct AI {
	int backwards;
};

union Things {
	struct Rock   rock;
	struct Player player;
	struct AI     ai;
};

struct Widget {
	struct Widget *next;
	int model;
	struct Vector3f x;
	int moving;
	int (*update)(struct Widget *w, float time);
	/* enum Type type; <- update stores type */
	union Things t;
};

int update_player(struct Widget *w, float time);
int update_ai(struct Widget *w, float time);

/* public */

/** constructor */
struct Widget *Widget(const enum Type type, const int model, ...) {
	va_list ap;
	struct Widget *widget;

	if(model <= 0) return 0;
	if(!(widget = malloc(sizeof(struct Widget)))) {
		perror("Widget constructor");
		Widget_(&widget);
		return 0;
	}
	widget->next = 0;
	widget->model = model;
	Vector3fZero(&widget->x);
	widget->moving = 0;
	widget->update = 0;
	/*widget->type   = type;*/
	va_start(ap, model);
	switch(type) {
		case t_rock:
			Vector3fZero(&widget->t.rock.v);
			break;
		case t_player:
			widget->update = &update_player;
			widget->t.player.input = va_arg(ap, struct Input *);
			Vector3fZero(&widget->t.player.v);
			widget->t.player.hp = 100;
			break;
		case t_ai:
			widget->update = &update_ai;
			widget->t.ai.backwards = 0;
			break;
		default:
			fprintf(stderr, "Widget: unrecognised type.\n");
			Widget_(&widget);
			return 0;
	}
	va_end(ap);
	fprintf(stderr, "Widget: new, #%p.\n", (void *)widget);

	return widget;
}

/** destructor */
void Widget_(struct Widget **widgetptr) {
	struct Widget *widget, *next;

	if(!widgetptr) return;
	widget = *widgetptr;
	while(widget) {
		next = widget->next;
		fprintf(stderr, "~Widget: erase, #%p.\n", (void *)widget);
		free(widget);
		widget = next;
	}
	*widgetptr = 0;
}

/** accessors */
struct Widget *WidgetGetNext(const struct Widget *widget) {
	if(!widget) return 0;
	return widget->next;
}

/*enum Type WidgetGetType(const struct Widget *w) { return w ? w->type : 0; }*/

int WidgetGetModel(const struct Widget *widget) {
	if(!widget) return 0;
	return widget->model;
}

struct Vector3f *WidgetGetX(struct Widget *widget) {
	if(!widget) return 0;
	return &widget->x;
}

int WidgetLink(struct Widget *w, struct Widget **head_ptr) {
	struct Widget *a;

	if(!w || w->next || !head_ptr) return 0;
	/* the list is empty */
	if(!(a = *head_ptr)) { *head_ptr = w; return -1; }
	/* get the last element */
	while(a->next) a = a->next;
	/* insert */
	a->next = w;
	return -1;
}

#define STRINGSIZE 32

char *WidgetToString(const struct Widget *w) {
	static char toString[STRINGSIZE];
	snprintf(toString, STRINGSIZE, "#%p", (void *)w);
	return toString;
}

void WidgetPrint(const struct Widget *w) {
	struct Widget *a;
	printf("Widgets: ");
	for(a = (struct Widget *)w; a; a = a->next) printf("%s->", WidgetToString(a));
	printf("null\n");
}

void WidgetUpdate(struct Widget *w, float time) {
	for( ; w; w = w->next) {
		if(!w->update) continue;
		w->update(w, time);
	}
}

int update_player(struct Widget *w, float time) {
	InputUpdate(w->t.player.input, w);
	return -1;
}

int update_ai(struct Widget *w, float time) {
	if(!w->t.ai.backwards) {
		w->x.x[0] += time;
		if(w->x.x[0] > 10) w->t.ai.backwards = -1;
	} else {
		w->x.x[0] -= time;
		if(w->x.x[0] < 0) w->t.ai.backwards = 0;
	}
	return -1;
}
