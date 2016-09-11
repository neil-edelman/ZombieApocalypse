struct Obj;

struct Obj *Obj(const char *fn);
void Obj_(struct Obj **objptr);
void ObjInfo(const struct Obj *o);
char *ObjGetSkin(const struct Obj *o);
int ObjGetNumCoords(const struct Obj *o);
void ObjDraw(const struct Obj *o);
