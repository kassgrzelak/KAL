#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"
#include "VM.h"

static void repl()
{
	printf(" _  __   _   _     \n");
	printf("| |/ /  /_\\ | |   \n");
	printf("| ' <  / _ \\| |__  K Assembly Language\n");
	printf("|_|\\_\\/_/ \\_\\____| v0.1.0\n");
	printf("\nPress return without typing anything to exit.\n\n");

	char line[1024];
	VM vm;

	for (;;)
	{
		printf("> ");

		// Exit if fgets fails or user presses enter without typing anything (only a newline in line).
		if (!fgets(line, sizeof(line), stdin) || line[1] == '\0')
		{
			printf("\n");
			break;
		}

		Bytecode code;
		size_t jumpTable[256];
		initBytecode(&code);
		compile(&code, jumpTable, line);

		for (int i = 0; i < code.count; ++i)
			printf("%d ", code.code[i]);
	}
}

static char* readFile(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Could not open file '%s'.\n", path);
		exit(74);
	}

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);
	rewind(file);

	char* buffer = malloc(fileSize + 1);
	if (buffer == NULL)
	{
		fprintf(stderr, "Not enough memory to read '%s'.\n", path);
		exit(74);
	}

	const size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize)
	{
		fprintf(stderr, "Couldn't read file '%s'.\n", path);
		exit(74);
	}
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

static void runFile(const char* path)
{
	char* source = readFile(path);
	free(source);
}

int main(const int argc, const char* argv[])
{
	if (argc == 1)
		repl();
	else if (argc == 2)
		runFile(argv[1]);
	else
	{
		fprintf(stderr, "Usage: kal [path]\n");
		exit(64);
	}

	return 0;
}