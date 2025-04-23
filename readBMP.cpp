#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Функция для чтения 2-байтового значения из файла
unsigned short Read2(FILE* f) {
    unsigned char buf[2];
    fread(buf, 1, 2, f);
    return buf[0] | (buf[1] << 8);
}

// Функция для чтения 4-байтового значения из файла
unsigned int Read4(FILE* f) {
    unsigned char buf[4];
    fread(buf, 1, 4, f);
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// Структура для заголовка BMP файла
struct bmp_file_header {
    char type[2];
    unsigned int size;    // Размер файла
    unsigned short reserved1; // Зарезервировано
    unsigned short reserved2; // Зарезервировано
    unsigned int offBits; // Смещение до пиксельных данных
};

// Структура для информационного заголовка BMP
struct bmp_info_header {
    unsigned int size;        // Размер этого заголовка
    int width;                // Ширина изображения в пикселях
    int height;               // Высота изображения в пикселях
    unsigned short planes;    // Число плоскостей
    unsigned short bitCount;  // Бит на пиксель
    unsigned int compression; // Тип сжатия
    unsigned int sizeImage;   // Размер пиксельных данных
    int xpixPerMeter;         // Горизонтальное разрешение
    int ypixPerMeter;         // Вертикальное разрешение
    unsigned int clrUsed;     // Число используемых цветов
    unsigned int clrImportant;// Число важных цветов
};

// Функция для чтения BMP файла (только 1-битные монохромные)
static unsigned char* read_bmp(const char* fname, int* _w, int* _h) {
    FILE* f = fopen(fname, "rb");  // Открываем файл в бинарном режиме
    if (!f) {
        printf("Не удается открыть файл %s\n", fname);
        return NULL;
    }

    struct bmp_file_header fh;
    struct bmp_info_header ih;

    // Читаем заголовок файла
    fread(fh.type, 1, 2, f); 
    fh.size = Read4(f);        // Размер файла
    fh.reserved1 = Read2(f);   // Зарезервированные поля
    fh.reserved2 = Read2(f);
    fh.offBits = Read4(f);     // Смещение до данных изображения

    // Читаем информационный заголовок
    ih.size = Read4(f);        // Размер заголовка
    ih.width = Read4(f);       // Ширина
    ih.height = Read4(f);      // Высота
    ih.planes = Read2(f);      // Число плоскостей
    ih.bitCount = Read2(f);    // Бит на пиксель
    ih.compression = Read4(f); // Тип сжатия
    ih.sizeImage = Read4(f);   // Размер данных изображения
    ih.xpixPerMeter = Read4(f); // Горизонтальное разрешение
    ih.ypixPerMeter = Read4(f); // Вертикальное разрешение
    ih.clrUsed = Read4(f);     // Число используемых цветов
    ih.clrImportant = Read4(f);// Число важных цветов

    // Проверяем, что это BMP файл
    if (fh.type[0] != 'B' || fh.type[1] != 'M') {
        printf("Это не BMP-файл\n");
        fclose(f);
        return NULL;
    }
    // Проверяем, что изображение 1-битное
    if (ih.bitCount != 1) {
        printf("Поддерживается только 1-bit BMP\n");
        fclose(f);
        return NULL;
    }

    int w = ih.width, h = ih.height;
    // Вычисляем размер строки с выравниванием по 4 байта
    int lineSize = ((w + 31) / 32) * 4;
    int fileSize = lineSize * h;

    // Выделяем память
    unsigned char* data = (unsigned char*)malloc(fileSize);
    unsigned char* img = (unsigned char*)malloc(w * h);

    if (!data || !img) {
        printf("Не удалось выделить память\n");
        free(data); free(img); fclose(f);
        return NULL;
    }

    // Переходим к началу пиксельных данных и читаем их
    fseek(f, fh.offBits, SEEK_SET);
    fread(data, 1, fileSize, f);

    // Декодируем пиксели (1 бит на пиксель)
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            int byteIndex = j * lineSize + i / 8;  // Индекс байта
            int bitIndex = 7 - (i % 8);           // Индекс бита в байте
            // Извлекаем бит и сохраняем в изображении (инвертируем Y)
            img[(h - j - 1)*w + i] = (data[byteIndex] >> bitIndex) & 1;
        }
    }

    // Возвращаем размеры через параметры
    *_w = w; *_h = h;

    free(data);  // Освобождаем сырые данные
    fclose(f);   // Закрываем файл

    return img;  // Возвращаем декодированное изображение
}

int main() {
    int w, h;
    // Читаем BMP файл (замените путь на ваш)
    unsigned char* img = read_bmp("C:\\file.bmp", &w, &h);

    if (!img)
        return 1;

    // Выводим изображение в консоль (пробел для 1, '*' для 0)
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++)
            printf("%c ", img[j * w + i] ? ' ' : '*');
        printf("\n");
    }

    free(img);  // Освобождаем память
    return 0;
}