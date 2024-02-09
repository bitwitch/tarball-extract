#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "zlib.h"

#include "microtar.h"

#define CHUNK_SIZE 262144

static int default_file_write(mtar_t *tar, const void *data, unsigned size) {
  size_t res = fwrite(data, 1, size, tar->stream);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int default_file_read(mtar_t *tar, void *data, unsigned size) {
  size_t res = fread(data, 1, size, tar->stream);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int default_file_seek(mtar_t *tar, unsigned offset) {
  int res = fseek(tar->stream, offset, SEEK_SET);
  return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static int default_file_close(mtar_t *tar) {
  fclose(tar->stream);
  return MTAR_ESUCCESS;
}

static int write_entire_file(char *path, char *buf, size_t size) {
	FILE *file = fopen(path, "wb");
	if (!file) {
		return MTAR_EOPENFAIL;
	}
	size_t bytes_written = fwrite(buf, 1, size, file);
	fclose(file);
	return (bytes_written == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int create_directory(char *path) {
	BOOL ok = CreateDirectoryA(path, NULL);
	return ok ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

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

int untar(FILE *file) {
	if (fseek(file, 0, SEEK_SET) != 0) {
		return MTAR_ESEEKFAIL;
	}

	mtar_header_t h;
	mtar_t tar = {0};
	tar.write = default_file_write;
	tar.read = default_file_read;
	tar.seek = default_file_seek;
	tar.close = default_file_close;
	tar.stream = file;

	int err = mtar_read_header(&tar, &h);
	while (err == MTAR_ESUCCESS) { 
		printf("%s (%d bytes)\n", h.name, h.size);

		if (h.type == MTAR_TDIR) {
			create_directory(h.name);
		} else if (h.type == MTAR_TREG) {
			char *buf = calloc(1, h.size + 1);
			mtar_read_data(&tar, buf, h.size);
			err = write_entire_file(h.name, buf, h.size);
			free(buf);
			if (err != MTAR_ESUCCESS) {
				break;
			}
		} else {
			err = MTAR_EREADFAIL;
			break;
		}

		mtar_next(&tar);
		err = mtar_read_header(&tar, &h);
	}
	
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: %s FILEPATH\n", argv[0]);
		return 1;
	}

	int result = 0;
	char *in_path = argv[1];
	char *out_path = "_temp_tarfile_";
	FILE *out_file = NULL;

	gzFile in_file = gzopen(in_path, "rb");
	if (!in_file) {
		fprintf(stderr, "Failed to open file: %s\n", in_path);
		result = 1;
		goto cleanup;
	}

	if (gzbuffer(in_file, CHUNK_SIZE) == -1) {
		fprintf(stderr, "Failed to grow gzbuffer size to: %d\n", CHUNK_SIZE);
		result = 1;
		goto cleanup;
	}

	out_file = fopen(out_path, "wb+");
	if (!out_file) {
		fprintf(stderr, "Failed to open file: %s\n", out_path);
		result = 1;
		goto cleanup;
	}

	int err = gzip_decompress(in_file, out_file);
	if (err != Z_OK) {
		fprintf(stderr, "gzip_decompress: %s\n", gzerror(in_file, &err));
		result = 1;
		goto cleanup;
	}

	err = untar(out_file);
	if (err != MTAR_ESUCCESS) {
		fprintf(stderr, "untar: %s\n", mtar_strerror(err));
		result = 1;
	}

cleanup:
	if (in_file) {
		gzclose(in_file);
	}
	if (out_file) {
		fclose(out_file);
		remove(out_path);
	}
	return 0;
}


