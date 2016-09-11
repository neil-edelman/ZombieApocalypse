struct Widget;

enum Type {
	t_rock,
	t_player,
	t_ai
};

struct Widget *Widget(const enum Type type, const int list, ...);
void Widget_(struct Widget **widgetptr);
struct Widget *WidgetGetNext(const struct Widget *widget);
enum Type WidgetGetType(const struct Widget *w);
int WidgetGetModel(const struct Widget *widget);
struct Vector3f *WidgetGetX(struct Widget *widget);
int WidgetLink(struct Widget *w, struct Widget **head_ptr);
char *WidgetToString(const struct Widget *w);
void WidgetPrint(const struct Widget *w);
void WidgetUpdate(struct Widget *w, float time);
