#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "zlib.h"

#include "microtar.h"

#define CHUNK_SIZE 262144

static int gzip_write(mtar_t *tar, const void *data, unsigned size) {
	(void)tar; (void)data; (void)size;
	// write should not get called for uncompressing tarballs
	assert(0 && "not implemented");
	return 0;
}

static int gzip_read(mtar_t *tar, void *data, unsigned size) {
	unsigned res = gzread(tar->stream, data, size);
	return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int gzip_seek(mtar_t *tar, unsigned offset) {
	// NOTE(shaw): zlib docs say that if the file is opened for reading, this
	// function is emulated but can be extremely slow 
	// see https://www.zlib.net/manual.html#Gzip
	z_off_t res = gzseek(tar->stream, offset, SEEK_SET);
	return (res == -1) ? MTAR_ESEEKFAIL: MTAR_ESUCCESS;
}

static int gzip_close(mtar_t *tar) {
  int res = gzclose(tar->stream);
  return res == Z_OK ? MTAR_ESUCCESS : MTAR_EFAILURE;
}

// int gzip_decompress(gzFile in_file, FILE *out_file) {
	// char out[CHUNK_SIZE];
	// int uncompressed_bytes = 0;
	// size_t bytes_written = 0;
	// int result = Z_OK;

	// for (;;) {
		// uncompressed_bytes = gzread(in_file, out, CHUNK_SIZE);
		// if (uncompressed_bytes == -1) {
			// result = Z_STREAM_ERROR;
			// break;
		// } 

		// bytes_written = fwrite(out, 1, uncompressed_bytes, out_file);
		// if (bytes_written != uncompressed_bytes || ferror(out_file)) {
			// result = Z_ERRNO;
			// break;
		// }

		// if (uncompressed_bytes < CHUNK_SIZE) {
			// break;
		// }
	// } 
	// return result;
// }

int tarball_extract(char *in_path, char *out_path) {
	(void)out_path;
	// char *p;
	mtar_header_t h;
	mtar_t tar = {0};
	tar.write = gzip_write;
	tar.read = gzip_read;
	tar.seek = gzip_seek;
	tar.close = gzip_close;

	tar.stream = gzopen(in_path, "rb");
	if (!tar.stream) {
		return MTAR_EOPENFAIL;
	}

	/* Read first header to check it is valid if mode is `r`
	   if (*mode == 'r') {
	   err = mtar_read_header(tar, &h);
	   if (err != MTAR_ESUCCESS) {
	   mtar_close(tar);
	   return err;
	   }
	   }
	   */

	int err = mtar_read_header(&tar, &h);
	while (err == MTAR_ESUCCESS) {
		printf("%s (%d bytes)\n", h.name, h.size);
		mtar_next(&tar);
		err = mtar_read_header(&tar, &h);
	}

	if (err == MTAR_ENULLRECORD) {
		err = MTAR_ESUCCESS;
	}

	return err;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: %s <INPUT FILEPATH> <OUTPUT FILEPATH>\n", argv[0]);
		return 1;
	}

	char *in_path = argv[1];
	char *out_path = argv[2];

	int result = tarball_extract(in_path, out_path);

	if (result != MTAR_ESUCCESS) {
		fprintf(stderr, "error: tarball_extract failed: %s\n", mtar_strerror(result));
		return 1;
	}

	return 0;

	// gzFile in_file = gzopen(in_path, "rb");
	// if (!in_file) {
		// fprintf(stderr, "Failed to open file: %s\n", in_path);
		// return 1;
	// }

	// if (gzbuffer(in_file, CHUNK_SIZE) == -1) {
		// fprintf(stderr, "Failed to grow gzbuffer size to: %d\n", CHUNK_SIZE);
		// gzclose(in_file);
		// return 1;
	// }

	// FILE *out_file = fopen(out_path, "wb");
	// if (!out_file) {
		// fprintf(stderr, "Failed to open file: %s\n", out_path);
		// gzclose(in_file);
		// return 1;
	// }


	// int result = gzip_decompress(in_file, out_file);

	// gzclose(in_file);
	// fclose(out_file);

	// switch (result) {
		// case Z_OK:
			// return 0;
		// case Z_STREAM_ERROR:
			// fprintf(stderr, "gzip_decompress: Z_STREAM_ERROR\n");
			// return 1;
		// case Z_ERRNO:
			// fprintf(stderr, "gzip_decompress: %s\n", strerror(errno));
			// return 1;
		// default:
			// fprintf(stderr, "gzip_decompress: unknown return code%d\n", result);
			// return 1;
	// }
}



