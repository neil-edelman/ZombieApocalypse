/* this is similar to an IntBuffer, StringBuffer (but not a Buffer) in Java
 but the size can be set in the constructor -Neil */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memcpy */
#include "Buffer.h"

static const unsigned initial = 16;

struct Buffer {
	void *data;
	size_t width;
	unsigned grow;
	unsigned size;
	unsigned capacity;
	unsigned iterator;
	int (*comparator)(const void *, const void *);
	void (*printer)(const void *);
};

static int expand(struct Buffer *b, unsigned cap);

/* public */

/** constructor */
struct Buffer *Buffer(const size_t width, int (*comparator)(const void *, const void *), void (*printer)(const void *)) {
	struct Buffer *buffer;
	void *data;

	if(width <= 0) return 0;
	if(!(buffer = malloc(sizeof(struct Buffer)))) {
		perror("Buffer constructor");
		Buffer_(&buffer);
		return 0;
	}
	buffer->data     = 0;
	buffer->width    = width;
	buffer->grow     = initial;
	buffer->size     = 0;
	buffer->capacity = 0;
	buffer->iterator = 0;
	buffer->comparator = comparator;
	buffer->printer  = printer;
	if(!(data = malloc(width*initial))) {
		perror("Buffer data");
		Buffer_(&buffer);
		return 0;
	}
	buffer->data     = data;
	buffer->capacity = initial;
	fprintf(stderr, "Buffer: new buffer (#%p,) %u of %u-sized, #%p.\n", data, (unsigned)initial, (unsigned)width, (void *)buffer);

	return buffer;
}

/** destructor */
void Buffer_(struct Buffer **buffer_ptr) {
	struct Buffer *buffer;

	if(!buffer_ptr || !(buffer = *buffer_ptr)) return;
	fprintf(stderr, "~Buffer: erase, #%p.\n", (void *)buffer);
	if(buffer->data) free(buffer->data);
	free(buffer);
	*buffer_ptr = buffer = 0;
}

size_t BufferGetWidth(const struct Buffer *b) {
	if(!b) return 0;
	return b->width;
}

int BufferSetGrow(struct Buffer *b, const int grow) {
	if(!b || grow == 0) return 0;
	b->grow = grow;
	return grow;
}

int BufferEnsureCapacity(struct Buffer *b, const unsigned min) {
	if(!b || min == 0) return 0;
	if(b->capacity >= min) return b->capacity;
	if(!expand(b, min)) return 0;
	return b->capacity;
}

void *BufferAdd(struct Buffer *b) {
	if(!b) return 0;
	if(b->size + 1 > b->capacity && !expand(b, b->capacity + b->grow)) return 0;
	return (char *)b->data + b->width*b->size++;
}

void BufferSub(struct Buffer *b) {
	if(!b) return;
	if(b->size) b->size--;
}

void *BufferAddMultiple(struct Buffer *b, const unsigned items) {
	void *a;
	if(!b || items <= 0) return 0;
	if(b->size + items >= b->capacity && !expand(b, b->size + items)) return 0;
	a = (char *)b->data + b->width*b->size;
	b->size += items;
	return a;
}

void BufferResetIterator(struct Buffer *b) {
	if(!b) return;
	b->iterator = 0;
}

void *BufferGetNext(struct Buffer *b) {
	if(!b) return 0;
	if(b->iterator >= b->size) {
		b->iterator = 0;
		return 0;
	}
	return (char *)b->data + b->width*b->iterator++;
}

unsigned BufferGetSize(const struct Buffer *b) {
	if(!b) return 0;
	return b->size;
}

void *BufferGet(struct Buffer *b, const unsigned item) {
	if(!b || item >= b->size) return 0;
	return (char *)b->data + b->width*item;
}

void BufferErase(struct Buffer *b) {
	if(!b) return;
	b->size = 0;
}

void BufferSort(struct Buffer *b) {
	if(!b || !b->comparator) return;
	qsort(b->data, b->size, b->width, b->comparator);
}

/** must be sorted */
void *BufferFind(const struct Buffer *b, const void *key) {
	if(!b || !key || !b->comparator) return 0;
	return bsearch(key, b->data, b->size, b->width, b->comparator);
}

void BufferPrint(struct Buffer *b) {
	char *a;
	printf("Buffer #%p with data %p: ", (void *)b, b->data);
	if(!b) {
		printf("(Null)\n");
		return;
	}
	while((a = BufferGetNext(b))) {
		if(b->printer) {
			b->printer(a);
		} else {
			int i;
			for(i = 0; i < b->width; i++) {
				char z = a[i];
				printf("%u ", z);
			}
		}
		printf(";");
	}
	printf("\n");
}

/* private */

static int expand(struct Buffer *b, unsigned cap) {
	void *data;
	fprintf(stderr, "Buffer: expanding #%p to %u.\n", (void *)b, cap);
	if(!(data = realloc(b->data, b->width*cap))) {
		perror("buffer");
		return 0;
	}
	b->data = data;
	b->capacity = cap;
	return cap;
}

/*

void *BufferGetPointer(const struct Buffer *b) {
	if(!b) return 0;
	return b->data;
}

unsigned BufferGetSize(const struct Buffer *b) {
	if(!b) return 0;
	return b->size;
}

void BufferOptimise(struct Buffer *b) {
	if(!b || b->len <= b->used) return;
	if(!realloc(b->data, b->size*b->used)) {
		perror("buffer");
		return;
	}
	b->len = b->used;
}
*/
