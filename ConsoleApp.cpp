#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ������� ��� ������ 2-��������� �������� �� �����
unsigned short Read2(FILE* f) {
	unsigned char buf[2];
	fread(buf, 1, 2, f);
	return buf[0] | (buf[1] << 8);
}

// ������� ��� ������ 4-��������� �������� �� �����
unsigned int Read4(FILE* f) {
	unsigned char buf[4];
	fread(buf, 1, 4, f);
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// ��������� ��� ��������� BMP �����
struct bmp_file_header {
	char type[2];
	unsigned int size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned int offBits;
};

// ��������� ��� ��������������� ��������� BMP
struct bmp_info_header {
	unsigned int size;
	int width;
	int height;
	unsigned short planes;
	unsigned short bitCount;
	unsigned int compression;
	unsigned int sizeImage;
	int xpixPerMeter;
	int ypixPerMeter;
	unsigned int clrUsed;
	unsigned int clrImportant;
};

// ��������� ��� ������������� BMP �����������
struct BMPImage {
	unsigned char** data;  // ��������� ������
	int width;
	int height;

	// ����� ��� �������� ����������� �� �����
	int(*loadFromFile)(struct BMPImage* img, const char* fname);

	// ����� ��� ������������ ������
	void(*release)(struct BMPImage* img);

	// ����� ��� ����������� ����������� � �������
	void(*display)(struct BMPImage* img);
};

// ����������� ��� �������� ���������� �������
void BMPImage_constructor(struct BMPImage* img, int width, int height) {
	img->width = width;
	img->height = height;

	// �������� ������ ��� ��������� �� ������
	img->data = (unsigned char**)malloc(height * sizeof(unsigned char*));

	// �������� ������ ��� ������ ������
	for (int i = 0; i < height; i++) {
		img->data[i] = (unsigned char*)malloc(width * sizeof(unsigned char));
	}
}

// ���������� ������ �������� �� �����
int BMPImage_loadFromFile(struct BMPImage* img, const char* fname) {
	FILE* f = fopen(fname, "rb");
	if (!f) {
		printf("�� ������� ������� ���� %s\n", fname);
		return 0;
	}

	struct bmp_file_header fh;
	struct bmp_info_header ih;

	fread(fh.type, 1, 2, f);
	fh.size = Read4(f);
	fh.reserved1 = Read2(f);
	fh.reserved2 = Read2(f);
	fh.offBits = Read4(f);

	ih.size = Read4(f);
	ih.width = Read4(f);
	ih.height = Read4(f);
	ih.planes = Read2(f);
	ih.bitCount = Read2(f);
	ih.compression = Read4(f);
	ih.sizeImage = Read4(f);
	ih.xpixPerMeter = Read4(f);
	ih.ypixPerMeter = Read4(f);
	ih.clrUsed = Read4(f);
	ih.clrImportant = Read4(f);

	if (fh.type[0] != 'B' || fh.type[1] != 'M') {
		printf("��� �� BMP-����\n");
		fclose(f);
		return 0;
	}

	if (ih.bitCount != 1) {
		printf("�������������� ������ 1-bit BMP\n");
		fclose(f);
		return 0;
	}

	int w = ih.width, h = ih.height;
	int lineSize = ((w + 31) / 32) * 4;
	int fileSize = lineSize * h;

	unsigned char* rawData = (unsigned char*)malloc(fileSize);
	if (!rawData) {
		printf("�� ������� �������� ������\n");
		fclose(f);
		return 0;
	}

	fseek(f, fh.offBits, SEEK_SET);
	fread(rawData, 1, fileSize, f);

	// ���������� ����������� ��� �������� ���������� �������
	BMPImage_constructor(img, w, h);

	// ��������� ��������� ������ �������
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			int byteIndex = j * lineSize + i / 8;
			int bitIndex = 7 - (i % 8);
			img->data[h - j - 1][i] = (rawData[byteIndex] >> bitIndex) & 1;
		}
	}

	free(rawData);
	fclose(f);

	return 1;
}

// ���������� ������ ������������ ������
void BMPImage_release(struct BMPImage* img) {
	if (img->data) {
		// ����������� ������ ������
		for (int i = 0; i < img->height; i++) {
			free(img->data[i]);
		}
		// ����������� ������ ����������
		free(img->data);
		img->data = NULL;
	}
	img->width = 0;
	img->height = 0;
}

// ���������� ������ ����������� �����������
void BMPImage_display(struct BMPImage* img) {
	for (int j = 0; j < img->height; j++) {
		for (int i = 0; i < img->width; i++) {
			printf("%c ", img->data[j][i] ? ' ' : '*');
		}
		printf("\n");
	}
}

// ������������� ��������� BMPImage
void BMPImage_init(struct BMPImage* img) {
	img->data = NULL;
	img->width = 0;
	img->height = 0;
	img->loadFromFile = BMPImage_loadFromFile;
	img->release = BMPImage_release;
	img->display = BMPImage_display;
}

int main() {
	struct BMPImage img;
	BMPImage_init(&img);

	if (!img.loadFromFile(&img, "file.bmp")) {
		return 1;
	}

	img.display(&img);
	img.release(&img);

	return 0;
}
