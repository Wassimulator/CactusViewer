#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CREATE_LIB 0

int main(int argc, char **argv) {
	FILE *file_in = fopen(argv[1], "rb");
	char file_out_path[128];
	sprintf(file_out_path, "%s.cpp", argv[1]);
	FILE *file_out = fopen(file_out_path, "wb");
	char* array_name = argv[2];
	if (!file_out || !file_in) return 1;

#if CREATE_LIB
	if (!array_name) return 1;
	fprintf(file_out,"unsigned char %s[] = { ", array_name);
#endif

	size_t count = 0;
	for (int c = fgetc(file_in); c != EOF; c = fgetc(file_in), count++) 
		fprintf(file_out, "0x%02x, ", c);

#if CREATE_LIB
	fprintf(file_out,"};\n");

	fprintf(file_out, "unsigned char* get_string_%s() { return %s; }", array_name, array_name);
	fprintf(file_out, "size_t get_string_size_%s() { return %zi; }", array_name, count);

	char command[128];
	sprintf(command, "cl /c \"%s\" /Fo:\"%s.obj\" && lib /OUT:\"%s.lib\" \"%s.obj\"", file_out_path, file_out_path, array_name, file_out_path);
	printf(command); printf("\n");
	system(command);
	sprintf(command, "del \"%s\" && del \"%s.obj\" && del \"%s.obj\"", file_out_path, file_out_path, argv[1]);
	system(command);
#endif

	fclose(file_out);
	fclose(file_in);
		
	printf("success\n");

	return 0;
}
