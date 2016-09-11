struct Bitmap *Bitmap();
void Bitmap_(struct Bitmap **bitmapPtr);
int BitmapGetWidth(const struct Bitmap *bmp);
int BitmapGetHeight(const struct Bitmap *bmp);
char *BitmapGetData(const struct Bitmap *bmp);	
int BitmapRead(struct Bitmap *bmp, const char *fn);
