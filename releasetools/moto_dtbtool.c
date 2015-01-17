/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* input_dtb[256];
const char* output_image;
int no_dtbs;

/* Maximum padding, 3 bytes */
const char padding[] = { 0, 0, 0 };

void print_usage()
{
	printf("Usage: moto_dtbtool -o output_dt_img [input_dtbs]\n");
}

size_t copy_file(FILE* in, FILE* out)
{
	char buffer[4096];
	size_t bytes;
	size_t total = 0;

	while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
	{
		total += bytes;
		fwrite(buffer, 1, bytes, out);
	}

	return total;
}

int main(int argc, char** argv)
{
	int i;
	int next_is_output = 0;

	/* Check arguments */
	for (i = 1; i < argc && no_dtbs < 256; i++)
	{
		if (!strcmp(argv[i], "-o"))
		{
			if (output_image)
			{
				/* Attempt to make second output */
				print_usage();
				return 1;
			}

			next_is_output = 1;
			continue;
		}

		if (next_is_output)
		{
			output_image = argv[i];
			next_is_output = 0;
			continue;
		}

		input_dtb[no_dtbs] = argv[i];
		no_dtbs++;
	}

	/* Check valitidy */
	if (no_dtbs < 1)
	{
		print_usage();
		return 1;
	}

	FILE* out_file = fopen(output_image, "w+");
	if (!out_file)
	{
		fprintf(stderr, "Cannot open %s for writing!\n", output_image);
		return 1;
	}

	for (i = 0; i < no_dtbs; i++)
	{
		size_t file_size;
		FILE* dtb_file = fopen(input_dtb[i], "r");

		if (!dtb_file)
		{
			fclose(out_file);
			fprintf(stderr, "Cannot open %s for reading!\n", input_dtb[i]);
			return 1;
		}

		file_size = copy_file(dtb_file, out_file);
		if (file_size % 4)
			fwrite(padding, 1, 4 - (file_size % 4), out_file);

		fclose(dtb_file);
	}

	fclose(out_file);
	return 0;
}
