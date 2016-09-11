#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include "Text.h"
#include "Vector.h"
#include "Map.h"

extern void (*vertex)(float, float, float);
extern void (*normal)(float, float, float);
extern void (*texel)(float, float);

/* public class */
struct Map {
	int         size;
	struct Grid **grid;
};

/* private class */
struct Grid {
	float altitude;
	struct Vector3f normal;
	int   specified;
};

/* constants */
static const float random_altitude = 0.3;

static void get_map_things(const struct Map *m, const int x, const int z, struct Vector3f *norm, float *alt);
static struct Grid *Grid(void);
static void Grid_(struct Grid **);
static void randomize_unused(struct Map *m);
static void generate_normals(struct Map *m);

/* public */

struct Map *Map(const char *fn) {
	struct Map  *m = 0;
	struct Text *txt;
	char        *str;

	if(!(txt = Text(fn))) {
		fprintf(stderr, "Map: couldn't load <%s>.\n", fn);
		return 0;
	}
	while(TextGetLine(txt)) {
		if(TextStartsWith(txt, "#")) {
			continue;
		} else if(TextStartsWith(txt, "size ")) {
			int size, x, z, i;
			if(m) {
				fprintf(stderr, "Map: <%s>, warning, idempotent, ignored further size.\n", fn);
				continue;
			}
			TextNextToken(txt);
			if(!(str = TextNextToken(txt))) continue;
			size = strtol(str, 0, 0);
			/* fixme: size > maxint^(1/2) */
			if(size < 3 || size > 1625) {
				fprintf(stderr, "Map: <%s>, size %d out of range.\n", fn, size);
				continue;
			}
			/* allocate the map */
			if(!(m = malloc(sizeof(struct Map) + sizeof(struct Grid *) * size * size))) {
				perror("Map constructor");
				Text_(&txt);
				Map_(&m);
				return 0;
			}
			m->size      = size;
			m->grid      = (struct Grid **)(m + 1);
			/* assign null pointers */
			for(i = 0; i < size * size; i++) m->grid[i] = 0;
			fprintf(stderr, "Map: <%s>, new %d, #%p.\n", fn, size, (void *)m);
			for(z = 0; z < size; z++) {
				for(x = 0; x < size; x++) {
					if(!(m->grid[z*size + x] = Grid())) {
						Text_(&txt);
						Map_(&m);
						return 0;
					}
				}
			}
		} else if(TextStartsWith(txt, "grid ")) {
			struct Grid *g;
			float alt;
			int x, z;
			if(!m) {
				fprintf(stderr, "Map: <%s>, grid commands before size ignored.\n", fn);
				continue;
			}
			TextNextToken(txt);
			if(!(str = TextNextToken(txt))) continue;
			x = strtol(str, 0, 0);
			if(!(str = TextNextToken(txt))) continue;
			z = strtol(str, 0, 0);
			if(!(str = TextNextToken(txt))) continue;
			alt = strtof(str, 0);
			/* audit */
			if(x < 0 || z < 0 || x > m->size-1 || z > m->size-1) {
				fprintf(stderr, "Map: <%s>, (%d, %d) off the map of size %d (zero-based!)\n", fn, x, z, m->size);
				continue;
			}
			g = m->grid[z*m->size + x];
			g->altitude  = alt;
			g->specified = -1;
		}
	}
	Text_(&txt);
	/* make sure we have a map after all that */
	if(!m) {
		fprintf(stderr, "Map: <%s>, no size.\n", fn);
		return 0;
	}
	randomize_unused(m);
	generate_normals(m);

	return m;
}

void Map_(struct Map **mapPtr) {
	int i;
	struct Map *map;
	
	if(!mapPtr || !(map = *mapPtr)) return;
	for(i = 0; i < map->size * map->size; i++) {
		if(map->grid[i]) Grid_(&map->grid[i]);
	}
	fprintf(stderr, "~Map: erase, #%p.\n", (void *)map);
	free(map);
	*mapPtr = map = 0;
}

int MapGetSize(const struct Map *map) {
	if(!map) return 0;
	return map->size;
}

float MapGetAltitude(const struct Map *map, const int x, const int z) {
	if(!map || x < 0 || z < 0 || x > map->size-1 || z > map->size-1) return 0;
	return map->grid[z*map->size + x]->altitude;
}

void MapSetAltitude(struct Map *map, const int x, const int z, const float a) {
	if(!map || x < 0 || z < 0 || x > map->size-1 || z > map->size-1) return;
	/* fixme: clamp to resonable values? */
	map->grid[z*map->size + x]->altitude = a;
}

struct Vector3f *MapGetNormal(const struct Map *map, const int x, const int z) {
	if(!map || x < 0 || z < 0 || x > map->size-1 || z > map->size-1) return 0;
	return &map->grid[z*map->size + x]->normal;
}

int MapUpdate(struct Map *m) {
	int size, x, z;
	struct Grid *g;
	
	if(!m) return 0;
	size = m->size;

	for(z = 0; z < size; z++) {
		for(x = 0; x < size; x++) {
			g = m->grid[z*size + x];
			/* do something */
		}
	}
	
	return -1;
}

/** this is called inside glBegin(GL_TRIANGLE_STRIP) */
int MapDraw(const struct Map *m, const struct Vector3f *eye) {
	int x, z;
	const int x_halfsize = 5, z_halfsize = 5;
	int x_min, x_max, z_min, z_max;
	struct Vector3f norm;
	float alt = 0;

	if(!m || !eye) return 0;

	x_min = eye->x[0] - x_halfsize;
	x_max = eye->x[0] + x_halfsize;
	z_min = eye->x[2] - z_halfsize;
	z_max = eye->x[2] + z_halfsize;

	for(x = x_min, z = z_min; ; ) {
		/* line (x,z) */
		get_map_things(m, x, z, &norm, &alt);
		texel(x, z);
		normal(norm.x[0], norm.x[1], norm.x[2]);
		vertex(x, alt, z);
		/* to (x,z+1) */
		get_map_things(m, x, z+1, &norm, &alt);
		texel(x, z+1);
		normal(norm.x[0], norm.x[1], norm.x[2]);
		vertex(x, alt, z+1);
		/* done x? */
		if(x >= x_max) {
			/* done z? */
			if(++z >= z_max) break;
			/* degenerate triangles (4); fixme: we'd only need 2 if we reversed */
			vertex(x, alt, z);
			x = x_min;
			get_map_things(m, x, z, &norm, &alt);
			vertex(x, alt, z);
			vertex(x, alt, z);
			vertex(x, alt, z);
		} else {
			x++;
		}
	}
	return -1;
}

/** this is an inefficent helper called by MapDraw */
static void get_map_things(const struct Map *m, const int x, const int z, struct Vector3f *norm, float *alt) {
	if(x < 0 || x >= m->size || z < 0 || z >= m->size) {
		Vector3fSerial(norm, 0, 1, 0);
		*alt = 0;
	} else {
		struct Grid *g = m->grid[(z)*m->size + x];
		*norm = g->normal;
		*alt  = g->altitude;
	}
}

int MapDraw_(const struct Map *m, const struct Vector3f *eye) {
	struct Grid *g;
	int size, x = 0, z = 0;
	float xt = 0, zt = 0, dt;
	
	if(!m) return 0;
	size = m->size;
	dt = 1 / (float)(size-1);
	for( ; ; ) {
		/* line (x,z)-(x,z+1), since x is incremented we have a grid */
		g = m->grid[(z)*size + x];
		texel(xt, zt);
		normal(g->normal.x[0], g->normal.x[1], g->normal.x[2]);
		vertex(x, g->altitude, z);
		g = m->grid[(z+1)*size + x];
		texel(xt, zt+dt);
		normal(g->normal.x[0], g->normal.x[1], g->normal.x[2]);
		vertex(x, g->altitude, z+1);
		/* done x? */
		if(x == size-1) {
			/* done z? */
			if(++z == size-1) break;
			zt += dt;
			/* degenerate triangles (4); fixme: we'd only need 2 if we reversed */
			vertex(x, m->grid[z*size + x]->altitude, z);
			vertex(0, m->grid[z*size]->altitude, z);
			vertex(0, m->grid[z*size]->altitude, z);
			vertex(0, m->grid[z*size]->altitude, z);
			x = 0;
			xt = 0;
		} else {
			x++;
			xt += dt;
		}
	}
	return -1;
}

/* private class */

static struct Grid *Grid(void) {
	struct Grid *g;
	
	if(!(g = malloc(sizeof(struct Grid)))) {
		perror("Grid constructor");
		Grid_(&g);
		return 0;
	}
	g->altitude  = 0;
	Vector3fSerial(&g->normal, 0, 1, 0);
	g->specified = 0;

	return g;
}

static void Grid_(struct Grid **gPtr) {
	struct Grid *g;
	
	if(!gPtr || !(g = *gPtr)) return;
	free(g);
	*gPtr = g = 0;
}

static void randomize_unused(struct Map *m) {
	struct Grid *g;
	int x, z;
	for(z = 0; z < m->size; z++) {
		for(x = 0; x < m->size; x++) {
			g = m->grid[z*m->size + x];
			if(g->specified) continue;
			g->altitude = (float)rand() / RAND_MAX * random_altitude;
		}
	}
	/* do some smoothing */
}

static void generate_normals(struct Map *m) {
	struct Grid *g;
	struct Vector3f n, ntot;
	int x, z;
	/* this gets four line segments, y=1, and averages them */
	for(z = 0; z < m->size; z++) {
		for(x = 0; x < m->size; x++) {
			g = m->grid[z*m->size + x];
			Vector3fZero(&ntot);
			if(x > 0) {
				n.x[0] = m->grid[z*m->size + (x-1)]->altitude - g->altitude;
				n.x[1] = 1;
				n.x[2] = 0;
				Vector3fAddTo(&ntot, &n);
			}
			if(z > 0) {
				n.x[0] = 0;
				n.x[1] = 1;
				n.x[2] = m->grid[(z-1)*m->size + x]->altitude - g->altitude;
				Vector3fAddTo(&ntot, &n);
			}
			if(x < m->size-1) {
				n.x[0] = g->altitude - m->grid[z*m->size + (x+1)]->altitude;
				n.x[1] = 1;
				n.x[2] = 0;
				Vector3fAddTo(&ntot, &n);
			}
			if(z < m->size-1) {
				n.x[0] = 0;
				n.x[1] = 1;
				n.x[2] = g->altitude - m->grid[(z+1)*m->size + x]->altitude;
				Vector3fAddTo(&ntot, &n);
			}
			Vector3fNormalise(&ntot);
			Vector3fSet(&g->normal, &ntot);
			/* printf("at %d,%d: %f %s\n", x, z, g->altitude, Vector3fToString(&g->normal)); */
		}
	}
}
