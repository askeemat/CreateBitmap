#include <stdio.h>
#include <stdlib.h>

#define NUM_COLOR_TABLE 256
#define ALIGN(x,a) ( ( (x)+(a)-1)&~( (a)-1) )

struct bitmapFileHeader {
	unsigned short bfType;
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
} __attribute__((packed));

struct bitmapInfoHeader {
    unsigned long  biSize;
    long           biWidth;
    long           biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImportant;
} __attribute__((packed));

struct rgbColorTable {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} __attribute__((packed));

union fileType {
	unsigned short short_expression;
	unsigned char char_expression[2];
};



void usage(void)
{
	printf("Usage:\n");
	printf("create_bitmap [in_binfile] [out_bmpfile] [width] [height]\n");
}

int main(int argc, char **argv)
{
	char *in_binfile, *out_bmpfile;
	char *linebuf_out;
	int width, height, aligned_width;
	int i, ret;
	FILE *fp_in, *fp_out;
	union fileType ftype;
	struct bitmapFileHeader fileheader;
	struct bitmapInfoHeader infoheader;
	struct rgbColorTable coltable[NUM_COLOR_TABLE];
	
	if (argc != 5) {
		printf ("argument is wrong.\n");
		usage();
		exit(EXIT_FAILURE);
	}
	
	in_binfile = argv[1];
	out_bmpfile = argv[2];
	width = atoi(argv[3]);
	height = atoi(argv[4]);

	ftype.char_expression[0] = 'B';
	ftype.char_expression[1] = 'M';
	fileheader.bfType = ftype.short_expression;

	aligned_width = ALIGN(width, 4);
	fileheader.bfSize = sizeof(struct bitmapFileHeader) + sizeof(struct bitmapInfoHeader) +
		sizeof(struct rgbColorTable) * NUM_COLOR_TABLE + aligned_width * height;
	fileheader.bfReserved1 = 0;
	fileheader.bfReserved2 = 0;
	fileheader.bfOffBits = sizeof(struct bitmapFileHeader) + sizeof(struct bitmapInfoHeader) +
		sizeof(struct rgbColorTable) * NUM_COLOR_TABLE;

	infoheader.biSize = 40;
	infoheader.biWidth = width;
	infoheader.biHeight = height;
	infoheader.biPlanes = 1;
	infoheader.biBitCount = 8;
	infoheader.biCompression = 0;
	infoheader.biSizeImage = 0;
	infoheader.biXPixPerMeter = 0;
	infoheader.biYPixPerMeter = 0;
	infoheader.biClrUsed = 0;
	infoheader.biClrImportant = 0;

	for (i = 0; i < NUM_COLOR_TABLE; i++) {
		coltable[i].rgbRed = i;
		coltable[i].rgbGreen = i;
		coltable[i].rgbBlue = i;
		coltable[i].rgbReserved = 0;
	}

	linebuf_out = (char *)calloc(aligned_width, sizeof(char));
	if (linebuf_out == NULL) {
		fprintf(stderr, "Cannot allocate memory(linebuf_out).\n");
		exit(EXIT_FAILURE);
	}

	if ((fp_in = fopen(in_binfile, "r")) == NULL) {
		fprintf(stderr, "Cannot open input file.\n");
		free(linebuf_out);
		exit(EXIT_FAILURE);
	}

	if ((fp_out = fopen(out_bmpfile, "w")) == NULL) {
		fprintf(stderr, "Cannot open output file.\n");
		fclose(fp_in);
		free(linebuf_out);
		exit(EXIT_FAILURE);
	}

	ret = fwrite((void *)&fileheader, sizeof(struct bitmapFileHeader), 1, fp_out);
	if (ret != 1) {
		fprintf(stderr, "write error\n");
	}

	ret = fwrite((void *)&infoheader, sizeof(struct bitmapInfoHeader), 1, fp_out);
	if (ret != 1) {
		fprintf(stderr, "write error\n");
	}
	
	ret = fwrite((void *)coltable, sizeof(struct rgbColorTable), NUM_COLOR_TABLE, fp_out);
	if (ret != NUM_COLOR_TABLE) {
		fprintf(stderr, "write error\n");
	}

	for (i = 0; i < height; i++) {
		ret = fread((void *)linebuf_out, sizeof(char), width, fp_in);
		if (ret != width) {
			fprintf(stderr, "read error\n");
		}
		ret = fwrite((void *)linebuf_out, sizeof(char), aligned_width, fp_out);
		if (ret != aligned_width) {
			fprintf(stderr, "write error\n");
		}
	}
	fclose(fp_out);
	fclose(fp_in);
	free(linebuf_out);

	printf("Finish\n");
}
