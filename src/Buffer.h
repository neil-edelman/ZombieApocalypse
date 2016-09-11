struct Buffer;

struct Buffer *Buffer(const size_t width, int (*comparator)(const void *, const void *), void (*printer)(const void *));
void Buffer_(struct Buffer **bufferptr);
size_t BufferGetWidth(const struct Buffer *b);
int BufferSetGrow(struct Buffer *b, const int grow);
int BufferEnsureCapacity(struct Buffer *b, const unsigned min);
void *BufferAdd(struct Buffer *b);
void BufferSub(struct Buffer *b);
void *BufferAddMultiple(struct Buffer *b, const unsigned items);
void BufferResetIterator(struct Buffer *b);
void *BufferGetNext(struct Buffer *b);
unsigned BufferGetSize(const struct Buffer *b);
void *BufferGet(struct Buffer *b, const unsigned item);
void BufferErase(struct Buffer *b);
void BufferSort(struct Buffer *b);
void *BufferFind(const struct Buffer *b, const void *key);
void BufferPrint(struct Buffer *b);
