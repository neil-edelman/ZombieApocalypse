#include <stdlib.h> /* malloc, free */
#include <stdio.h>  /* fprintf */
/*#include <errno.h> <- doesn't like this on my compiler */  /* errno */
#include <stdint.h>
#include "Buffer.h"
#include "Bitmap.h"

/* http://en.wikipedia.org/wiki/BMP_file_format */

/* bitmap format: */
/* -> magic 'BM' (we'll ignore other magiks) */
struct Header {
	uint32_t size;
	uint16_t creator1;
	uint16_t creator2;
	uint32_t offset;
};
/* -> uint32_t 40 (we'll ignore other formats) */
struct Info {
	int32_t  width;
	int32_t  height;
	uint16_t nplanes;
	uint16_t bpp;
	uint32_t compress;
	uint32_t bmp_byte;
	int32_t  hres;
	int32_t  vres;
	uint32_t ncolors;
	uint32_t nimpcolors;
};

struct Bitmap {
	struct Header head;
	struct Info   info;
	struct Buffer *buf;
};

/* compression types */
enum Compress {
	BI_RGB = 0,
	BI_RLE8 = 1,
	BI_RLE4 = 2,
	BI_BITFIELDS = 3,
	BI_JPEG = 4,
	BI_PNG = 5
};

/* private */
int read_bitmap(struct Bitmap *bmp, FILE *fp, const char *fn);

/* public */

/** buf must be alive while the Bitmap's alive, and it is erased! */
struct Bitmap *Bitmap(const char *fn, struct Buffer */*<char>*/buf) {
	struct Bitmap *bmp;
	FILE *fp;

	if(!fn || !buf) return 0;
	if(BufferGetWidth(buf) != sizeof(char)) {
		fprintf(stderr, "Bitmap: <%s> called with inapproprate buffer.\n", fn);
		return 0;
	}
	if(!(bmp = malloc(sizeof(struct Bitmap)))) {
		perror("Bitmap constructor");
		Bitmap_(&bmp);
		return 0;
	}
	bmp->buf = buf;
	fprintf(stderr, "Bitmap: new <%s>, #%p.\n", fn, (void *)bmp);
	if(!(fp = fopen(fn, "rb"))) {
		perror(fn);
		Bitmap_(&bmp);
		return 0;
	}
	BufferErase(buf);
	if(!read_bitmap(bmp, fp, fn)) {
		perror(fn);
		fclose(fp);
		Bitmap_(&bmp);
		return 0;
	}
	fclose(fp);

	return bmp;
}

void Bitmap_(struct Bitmap **bitmapPtr) {
	struct Bitmap *bmp;
	if(!bitmapPtr || !(bmp = *bitmapPtr)) return;
	fprintf(stderr, "~Bitmap: erase, #%p.\n", (void *)bmp);
	free(bmp);
	*bitmapPtr = bmp = 0;
}

int BitmapGetWidth(const struct Bitmap *bmp) {
	if(!bmp) return 0;
	return bmp->info.width;
}

int BitmapGetHeight(const struct Bitmap *bmp) {
	if(!bmp) return 0;
	return bmp->info.height;
}

char *BitmapGetData(const struct Bitmap *bmp) {
	if(!bmp) return 0;
	return BufferGet(bmp->buf, 0);
}

/* private */

int read_bitmap(struct Bitmap *bmp, FILE *fp, const char *fn) {
	uint32_t dibHeader;
	size_t bytes;
	void *ptr;
	char magic[2];

	/* magic */
	if(fread(magic, sizeof(char), 2, fp) != 2) return 0;
	if(magic[0] != 'B' || magic[1] != 'M') {
		fprintf(stderr, "%s: expeced 'BM,' unsuppored format, '%c%c'.\n", fn, magic[0], magic[1]);
		return 0;
	}
	/* file header */
	if(fread(&bmp->head, sizeof(struct Header), 1, fp) != 1) return 0;
	if(bmp->head.offset < 2 + sizeof(struct Header) + sizeof(uint32_t) + sizeof(struct Info)) {
		fprintf(stderr, "%s: offset reported %d doesn't make sense.\n", fn, bmp->head.offset);
		return 0;
	}
	/* DIB Header */
	if(fread(&dibHeader, sizeof(uint32_t), 1, fp) != 1) return 0;
	if(dibHeader != 40) {
		fprintf(stderr, "%s: DIB Header 40, bitmap not supported %d.\n", fn, dibHeader);
		return 0;
	}
	/* bitmap info header */
	if(fread(&bmp->info, sizeof(struct Info), 1, fp) != 1) return 0;
	if(bmp->info.width <= 0) {
		fprintf(stderr, "%s: width is %d? what does this mean?\n", fn, bmp->info.width);
		return 0;
	}
	/* fixme! also, padding! */
	if(bmp->info.height < 0) {
		bmp->info.height = -bmp->info.height;
	} else {
		fprintf(stderr, "%s: bitmap flipped in file; too lazy to reverse it.\n", fn);
	}
	if(bmp->info.nplanes != 1 || bmp->info.bpp != 24) {
		fprintf(stderr, "%s: bitmap must be 1 plane, 24 bpp, is %d, %d.\n", fn, bmp->info.nplanes, bmp->info.bpp);
		return 0;
	}
	if((enum Compress)bmp->info.compress != BI_RGB) {
		fprintf(stderr, "%s: bitmap must be uncompressed, compression %d.\n", fn, bmp->info.compress);
		return 0;
	}
	if(fseek(fp, bmp->head.offset, SEEK_SET)) return 0;
	bytes = bmp->info.width * bmp->info.height * 3;
	if(!(ptr = BufferAddMultiple(bmp->buf, bytes))) return 0;
	/* BGR */
	if(fread(ptr, sizeof(char), bytes, fp) != bytes) {
		fprintf(stderr, "%s: ran out of data.\n", fn);
		return 0;
	}
	fprintf(stderr, "%s: read %dx%d.\n", fn, bmp->info.width, bmp->info.height);

	return -1;
}
