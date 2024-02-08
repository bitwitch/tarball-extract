#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "zlib.h"

#define CHUNK_SIZE 262144

int gzip_decompress(gzFile in_file, FILE *out_file) {
	char out[CHUNK_SIZE];
	int uncompressed_bytes = 0;
	size_t bytes_written = 0;
	int result = Z_OK;

	for (;;) {
		uncompressed_bytes = gzread(in_file, out, CHUNK_SIZE);
		if (uncompressed_bytes == -1) {
			result = Z_STREAM_ERROR;
			break;
		} 

		bytes_written = fwrite(out, 1, uncompressed_bytes, out_file);
		if (bytes_written != uncompressed_bytes || ferror(out_file)) {
			result = Z_ERRNO;
			break;
		}

		if (uncompressed_bytes < CHUNK_SIZE) {
			break;
		}
	} 
	return result;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: %s <INPUT FILEPATH> <OUTPUT FILEPATH>\n", argv[0]);
		return 1;
	}

	char *in_path = argv[1];
	char *out_path = argv[2];

	gzFile in_file = gzopen(in_path, "rb");
	if (!in_file) {
		fprintf(stderr, "Failed to open file: %s\n", in_path);
		return 1;
	}

	if (gzbuffer(in_file, CHUNK_SIZE) == -1) {
		fprintf(stderr, "Failed to grow gzbuffer size to: %d\n", CHUNK_SIZE);
		gzclose(in_file);
		return 1;
	}

	FILE *out_file = fopen(out_path, "wb");
	if (!out_file) {
		fprintf(stderr, "Failed to open file: %s\n", out_path);
		gzclose(in_file);
		return 1;
	}

	int result = gzip_decompress(in_file, out_file);

	gzclose(in_file);
	fclose(out_file);

	switch (result) {
		case Z_OK:
			return 0;
		case Z_STREAM_ERROR:
			fprintf(stderr, "gzip_decompress: Z_STREAM_ERROR\n");
			return 1;
		case Z_ERRNO:
			fprintf(stderr, "gzip_decompress: %s\n", strerror(errno));
			return 1;
		default:
			fprintf(stderr, "gzip_decompress: unknown return code%d\n", result);
			return 1;
	}
}



