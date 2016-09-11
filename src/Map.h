struct Map;

struct Map *Map(const char *fn);
void Map_(struct Map **);
int MapGetSize(const struct Map *map);
float MapGetAltitude(const struct Map *map, const int x, const int z);
void MapSetAltitude(struct Map *map, const int x, const int z, const float a);
struct Vector3f *MapGetNormal(const struct Map *map, const int x, const int z);
int MapUpdate(struct Map *m);
int MapDraw(const struct Map *m, const struct Vector3f *eye);
