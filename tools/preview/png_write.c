#include "png_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static void put_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static int write_chunk(FILE *fp, const char tag[4], const uint8_t *data, uint32_t len) {
    uint8_t lenb[4];
    uint32_t crc;

    put_be32(lenb, len);
    if (fwrite(lenb, 1, 4, fp) != 4) return -1;
    if (fwrite(tag, 1, 4, fp) != 4) return -1;
    if (len && fwrite(data, 1, len, fp) != len) return -1;
    crc = crc32(0, (const Bytef *)tag, 4);
    if (len) crc = crc32(crc, data, len);
    put_be32(lenb, crc);
    return fwrite(lenb, 1, 4, fp) == 4 ? 0 : -1;
}

int png_write_rgb(const char *path, const uint8_t *rgb, int width, int height) {
    size_t stride = (size_t)width * 3u + 1u;
    size_t raw_n = stride * (size_t)height;
    uint8_t *raw;
    uint8_t *comp;
    uLongf comp_n;
    FILE *fp;
    uint8_t ihdr[13];
    int y;
    int rc = -1;

    if (!path || !rgb || width <= 0 || height <= 0) return -1;
    raw = (uint8_t *)malloc(raw_n);
    if (!raw) return -1;
    for (y = 0; y < height; y++) {
        raw[(size_t)y * stride] = 0;
        memcpy(raw + (size_t)y * stride + 1, rgb + (size_t)y * (size_t)width * 3u, (size_t)width * 3u);
    }
    comp_n = compressBound((uLong)raw_n);
    comp = (uint8_t *)malloc(comp_n);
    if (!comp) {
        free(raw);
        return -1;
    }
    if (compress2(comp, &comp_n, raw, (uLong)raw_n, 6) != Z_OK) {
        free(comp);
        free(raw);
        return -1;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        free(comp);
        free(raw);
        return -1;
    }
    if (fwrite("\x89PNG\r\n\x1a\n", 1, 8, fp) != 8) goto done;
    put_be32(ihdr, (uint32_t)width);
    put_be32(ihdr + 4, (uint32_t)height);
    ihdr[8] = 8;
    ihdr[9] = 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    if (write_chunk(fp, "IHDR", ihdr, 13) != 0) goto done;
    if (write_chunk(fp, "IDAT", comp, (uint32_t)comp_n) != 0) goto done;
    if (write_chunk(fp, "IEND", NULL, 0) != 0) goto done;
    rc = 0;
done:
    fclose(fp);
    free(comp);
    free(raw);
    return rc;
}
