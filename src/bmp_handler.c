#include "bmp_handler.h"

WORD read_word(const BYTE* data_to_read, unsigned int* const reading_offset) {
    data_to_read += *reading_offset;
    const WORD word_container = data_to_read[0] | (data_to_read[1] << BITS_IN_BYTE_NUM);
    *reading_offset += sizeof(WORD);
    return word_container;
}

DWORD read_dword(const BYTE* data_to_read, unsigned int* const reading_offset) {
    data_to_read += *reading_offset;
    const DWORD dword_container = data_to_read[0] | data_to_read[1] << BITS_IN_BYTE_NUM | data_to_read[2] << 2 * BITS_IN_BYTE_NUM | data_to_read[3] << 3 * BITS_IN_BYTE_NUM;
    *reading_offset += sizeof(DWORD);
    return dword_container;
}

LONG read_long(const BYTE* data_to_read, unsigned int* const reading_offset) {
    data_to_read += *reading_offset;
    const LONG long_container = data_to_read[0] | data_to_read[1] << BITS_IN_BYTE_NUM | data_to_read[2] << 2 * BITS_IN_BYTE_NUM | data_to_read[3] << 3 * BITS_IN_BYTE_NUM;
    *reading_offset += sizeof(LONG);
    return long_container;
}

void write_word(const WORD* const word_container, BYTE* data_to_write, unsigned int* const writing_offset) {
    data_to_write += *writing_offset;
    data_to_write[0] = (*word_container & 0x00ffu);
    data_to_write[1] = (*word_container & 0xff00u) >> BITS_IN_BYTE_NUM;
    *writing_offset += sizeof(WORD);
}

void write_dword(const DWORD* const dword_container, BYTE* data_to_write, unsigned int* const writing_offset) {
    data_to_write += *writing_offset;
    data_to_write[0] = (*dword_container & 0x000000ffu);
    data_to_write[1] = (*dword_container & 0x0000ff00u) >> BITS_IN_BYTE_NUM;
    data_to_write[2] = (*dword_container & 0x00ff0000u) >> 2 * BITS_IN_BYTE_NUM;
    data_to_write[3] = (*dword_container & 0xff000000u) >> 3 * BITS_IN_BYTE_NUM;
    *writing_offset += sizeof(DWORD);
}

void write_long(const LONG* const long_container, BYTE* data_to_write, unsigned int* const writing_offset) {
    data_to_write += *writing_offset;
    data_to_write[0] = (*long_container & 0x000000ffu);
    data_to_write[1] = (*long_container & 0x0000ff00u) >> BITS_IN_BYTE_NUM;
    data_to_write[2] = (*long_container & 0x00ff0000u) >> 2 * BITS_IN_BYTE_NUM;
    data_to_write[3] = (*long_container & 0xff000000u) >> 3 * BITS_IN_BYTE_NUM;
    *writing_offset += sizeof(LONG);
}

enum return_codes_t read_bmp_header(struct bmp_header_t* const header_container, FILE* const file_to_read) {
    message_handle("Reading BMP header... ", report);
    BYTE bmp_header[FULL_HEADER_SIZE];
    if (fread(bmp_header, sizeof(BYTE), FULL_HEADER_SIZE, file_to_read) != FULL_HEADER_SIZE) {
        message_handle("Failed to read BMP header\n", error);
        return runtime_error;
    }
    unsigned int reading_offset = 0;
    header_container->file_type = read_word(bmp_header, &reading_offset);
    header_container->file_size = read_dword(bmp_header, &reading_offset);
    header_container->reserved1 = read_word(bmp_header, &reading_offset);
    header_container->reserved2 = read_word(bmp_header, &reading_offset);
    header_container->bitmap_offset = read_dword(bmp_header, &reading_offset);
    header_container->dib_header_size = read_dword(bmp_header, &reading_offset);
    header_container->width = read_long(bmp_header, &reading_offset);
    header_container->height = read_long(bmp_header, &reading_offset);
    header_container->planes = read_word(bmp_header, &reading_offset);
    header_container->bpp = read_word(bmp_header, &reading_offset);
    header_container->compression = read_dword(bmp_header, &reading_offset);
    header_container->bitmap_size = read_dword(bmp_header, &reading_offset);
    header_container->horz_res = read_long(bmp_header, &reading_offset);
    header_container->vert_res = read_long(bmp_header, &reading_offset);
    header_container->colors_num = read_dword(bmp_header, &reading_offset);
    header_container->colors_req = read_dword(bmp_header, &reading_offset);
    message_handle("Success\n", report);
    return success;
}

enum return_codes_t write_bmp_header(const struct bmp_header_t* const header_container, FILE* const file_to_write) {
    BYTE bmp_header[FULL_HEADER_SIZE];
    unsigned int writing_offset = 0;
    write_word(&header_container->file_type, bmp_header, &writing_offset);
    write_dword(&header_container->file_size, bmp_header, &writing_offset);
    write_word(&header_container->reserved1, bmp_header, &writing_offset);
    write_word(&header_container->reserved2, bmp_header, &writing_offset);
    write_dword(&header_container->bitmap_offset, bmp_header, &writing_offset);
    write_dword(&header_container->dib_header_size, bmp_header, &writing_offset);
    write_long(&header_container->width, bmp_header, &writing_offset);
    write_long(&header_container->height, bmp_header, &writing_offset);
    write_word(&header_container->planes, bmp_header, &writing_offset);
    write_word(&header_container->bpp, bmp_header, &writing_offset);
    write_dword(&header_container->compression, bmp_header, &writing_offset);
    write_dword(&header_container->bitmap_size, bmp_header, &writing_offset);
    write_long(&header_container->horz_res, bmp_header, &writing_offset);
    write_long(&header_container->vert_res, bmp_header, &writing_offset);
    write_dword(&header_container->colors_num, bmp_header, &writing_offset);
    write_dword(&header_container->colors_req, bmp_header, &writing_offset);
    if (fwrite(bmp_header, sizeof(BYTE), FULL_HEADER_SIZE, file_to_write) != FULL_HEADER_SIZE) {
        message_handle("Failed to write BMP header to output file\n", error);
        return runtime_error;
    }
    return success;
}

enum return_codes_t verify_bmp_header(struct bmp_header_t* const header_container, FILE* bmp_file) {
    message_handle("Verifying BMP header... ", report);
    if (header_container->file_type != BMP_SIGN) {
        message_handle("Unsupported file type (not a BMP file according to file type)\n", error);
        return runtime_error;
    }
    if (fseek(bmp_file, 0, SEEK_END) != 0) {
        message_handle("Failed to reach the end of the given BMP file\n", error);
        return runtime_error;
    }
    const long bmp_file_size = ftell(bmp_file);
    if (fseek(bmp_file, FULL_HEADER_SIZE, SEEK_SET) != 0) {
        message_handle("Failed to reach the next byte after the end of the given BMP header\n", error);
        return runtime_error;
    }
    if (bmp_file_size == -1) {
        message_handle("Failed to calculate the size of the given BMP file\n", error);
        return runtime_error;
    }
    if (header_container->file_size != bmp_file_size) {
        message_handle("Unsupported file type (given file size does not match calculated file size)\n", error);
        return runtime_error;
    }
    if (header_container->reserved1 != 0 || header_container->reserved2 != 0) {
        message_handle("Unsupported file type (reserved fields must be set to zero)\n", error);
        return runtime_error;
    }
    if (header_container->dib_header_size != DIB_HEADER_SIZE) {
        message_handle("Unsupported file type (unsupported BMP version, only v3 is supported)\n", error);
        return runtime_error;
    }
    if (header_container->width <= 0) {
        message_handle("Unsupported file type (image width must be positive)\n", error);
        return runtime_error;
    }
    if (header_container->height == 0) {
        message_handle("Unsupported file type (image height must be non-zero)\n", error);
        return runtime_error;
    }
    if (header_container->planes != PLANES_NUM) {
        message_handle("Unsupported file type (planes number must be set to 1)\n", error);
        return runtime_error;
    }
    if (header_container->bpp != BPP_8BIT && header_container->bpp != BPP_24BIT) {
        message_handle("Unsupported file type (unsupported bpp, only 8- and 24-bit are supported)\n", error);
        return runtime_error;
    }
    if (header_container->compression != 0) {
        message_handle("Unsupported file type (compression detected, only uncompressed images are supported)\n", error);
        return runtime_error;
    }
    if (header_container->bitmap_size != 0) {
        message_handle("Detected non-zero bitmap size (given size will be ignored)\n", warning);
    }
    header_container->bitmap_size = BITMAP_SIZE_FORMULA(header_container->width, header_container->height, header_container->bpp);
    if ((header_container->colors_num == 0 || header_container->colors_num > MAX_8BIT_PALETTE_SIZE) && header_container->bpp == BPP_8BIT) {
        message_handle("Detected unrealistic palette size when using 8bpp (interpreted as maximum size possible)\n", warning);
        header_container->colors_num = MAX_8BIT_PALETTE_SIZE;
    }
    if (header_container->colors_num != 0 && header_container->bpp == BPP_24BIT) {
        message_handle("Unsupported file type (palette size must be set to zero when using 24bpp)\n", error);
        return runtime_error;
    }
    message_handle("Success\n", report);
    return success;
}

enum return_codes_t read_bmp_palette(BYTE** const palette_container, const DWORD* const colors_num, FILE* const file_to_read) {
    message_handle("Reading color palette... ", report);
    *palette_container = (BYTE*) malloc(*colors_num * sizeof(DWORD));
    if (*palette_container == NULL) {
        message_handle("Memory allocation for color palette failed\n", error);
        return runtime_error;
    }
    if (fread(*palette_container, sizeof(DWORD), *colors_num, file_to_read) != *colors_num) {
        message_handle("Failed to read color palette\n", error);
        free(*palette_container);
        return pixel_data_corruption;
    }
    message_handle("Success\n", report);
    return success;
}

enum return_codes_t write_bmp_palette(const BYTE* const palette_container, const DWORD* const colors_num, FILE* const file_to_write) {
    if (fwrite(palette_container, sizeof(DWORD), *colors_num, file_to_write) != *colors_num) {
        message_handle("Failed to write color palette to output file\n", error);
        return runtime_error;
    }
    return success;
}

enum return_codes_t read_bmp_pixels(BYTE** const pixels_container, const DWORD* const bitmap_size, FILE* const file_to_read) {
    message_handle("Reading pixel data... ", report);
    *pixels_container = (BYTE*) malloc(*bitmap_size);
    if (*pixels_container == NULL) {
        message_handle("Memory allocation for pixel data failed\n", error);
        return runtime_error;
    }
    if (fread(*pixels_container, 1, *bitmap_size, file_to_read) != *bitmap_size) {
        message_handle("Failed to read pixel data\n", error);
        free(*pixels_container);
        return pixel_data_corruption;
    }
    message_handle("Success\n", report);
    return success;
}

enum return_codes_t write_bmp_pixels(const BYTE* const pixels_container, const DWORD* const bitmap_size, FILE* const file_to_write) {
    if (fwrite(pixels_container, 1, *bitmap_size, file_to_write) != *bitmap_size) {
        message_handle("Failed to write pixel data to output file\n", error);
        return runtime_error;
    }
    return success;
}

enum return_codes_t verify_bmp_8bpp_pixels(const BYTE* const pixels_container, const struct bmp_header_t* const header_container) {
    message_handle("Verifying 8bpp pixel data... ", report);
    const unsigned int bytes_per_row = header_container->bitmap_size / abs(header_container->height), bytes_per_pixel = BPP_8BIT / BITS_IN_BYTE_NUM;
    for (unsigned int i = 0; i < (unsigned) abs(header_container->height); i++) {
        for (unsigned int j = 0; j < (unsigned long) header_container->width; j++) {
            const BYTE pxl = *(pixels_container + i * bytes_per_row + j * bytes_per_pixel);
            if (pxl >= header_container->colors_num) {
                message_handle("Pixels contain colors unspecified by given color palette\n", error);
                return pixel_data_corruption;
            }
        }
    }
    message_handle("Success\n", report);
    return success;
}

void close_file_urgently(FILE* const file_to_close) {
    if (fclose(file_to_close) != 0) {
        message_handle("Also failed to close BMP file\n", error);
    }
}

enum return_codes_t read_bmp_file(struct bmp_file_t* const bmp_container, const char filename[]) {
    FILE* file_to_read = fopen(filename, "rb");
    if (file_to_read == NULL) {
        message_handle("Failed to open given BMP file\n", error);
        return runtime_error;
    }
    if (read_bmp_header(&bmp_container->header, file_to_read) != success || verify_bmp_header(&bmp_container->header, file_to_read) != success) {
        close_file_urgently(file_to_read);
        return runtime_error;
    }
    enum return_codes_t return_code;
    if (bmp_container->header.bpp == BPP_8BIT) {
        return_code = read_bmp_palette(&bmp_container->palette, &bmp_container->header.colors_num, file_to_read);
        if (return_code != success) {
            close_file_urgently(file_to_read);
            return return_code;
        }
    } else {
        bmp_container->palette = NULL;
    }
    if (fseek(file_to_read, bmp_container->header.bitmap_offset, SEEK_SET) != 0) {
        message_handle("Failed to reach given bitmap offset\n", error);
        return runtime_error;
    }
    return_code = read_bmp_pixels(&bmp_container->bitmap, &bmp_container->header.bitmap_size, file_to_read);
    if (return_code != success) {
        if (bmp_container->palette != NULL) {
            free(bmp_container->palette);
        }
        close_file_urgently(file_to_read);
        return return_code;
    }
    if (fclose(file_to_read) != 0) {
        message_handle("Failed to close BMP file\n", error);
        return runtime_error;
    }
    if (bmp_container->header.bpp == BPP_8BIT) {
        return_code = verify_bmp_8bpp_pixels(bmp_container->bitmap, &bmp_container->header);
        if (return_code != success) {
            free(bmp_container->palette);
            free(bmp_container->bitmap);
            return return_code;
        }
    }
    return success;
}

enum return_codes_t write_bmp_file(struct bmp_file_t* const bmp_container, const char filename[]) {
    message_handle("Writing new BMP file... ", report);
    FILE* file_to_write = fopen(filename, "wb");
    if (file_to_write == NULL) {
        message_handle("Failed to create BMP file\n", error);
        return runtime_error;
    }
    if (write_bmp_header(&bmp_container->header, file_to_write) != success ||
        (bmp_container->header.bpp == BPP_8BIT && write_bmp_palette(bmp_container->palette, &bmp_container->header.colors_num, file_to_write) != success) ||
        write_bmp_pixels(bmp_container->bitmap, &bmp_container->header.bitmap_size, file_to_write) != success) {
        return runtime_error;
    }
    if (fclose(file_to_write) != 0) {
        message_handle("Failed to close output file\n", error);
        return runtime_error;
    }
    message_handle("Success\n", report);
    return success;
}

void dealloc_bmp_container(struct bmp_file_t* const bmp_container) {
    if (bmp_container->header.bpp == BPP_8BIT) {
        free(bmp_container->palette);
    }
    free(bmp_container->bitmap);
}
