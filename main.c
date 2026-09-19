#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"
#include "VM.h"

static void repl()
{
	// Kind of weird for an assembly language to have a REPL but it makes for quick testing.
	printf(" _  __   _   _     \n");
	printf("| |/ /  /_\\ | |   \n");
	printf("| ' <  / _ \\| |__  KAL Assembly Language\n");
	printf("|_|\\_\\/_/ \\_\\____|\n");
	printf("\nPress return without typing anything to exit.\n\n");

	char line[1024];
	VM vm;
	initVM(&vm);

	for (;;)
	{
		printf("\n> ");

		// Exit if fgets fails or user presses enter without typing anything (only a newline in line).
		if (!fgets(line, sizeof(line), stdin) || line[1] == '\0')
		{
			printf("\n");
			break;
		}

		interpret(&vm, line);
	}

	freeVM(&vm);
}

static char* readFile(const char* path)
{
	// Open file.
	FILE* file = fopen(path, "rb");
	if (file == NULL)
	{
		printf("Could not open file '%s'.\n", path);
		exit(74);
	}

	// Get size of file.
	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);
	rewind(file);

	// Allocate space to read file.
	char* buffer = malloc(fileSize + 2);
	if (buffer == NULL)
	{
		printf("Not enough memory to read '%s'.\n", path);
		exit(74);
	}

	// Read file.
	const size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize)
	{
		printf("Couldn't read file '%s'.\n", path);
		exit(74);
	}

	// Add a newline at end of file in case there wasn't one already otherwise our scanner will get upset.
	buffer[bytesRead] = '\n';
	buffer[bytesRead + 1] = '\0';

	// Close and return.
	fclose(file);
	return buffer;
}

static int runFile(const char* path)
{
	char* source = readFile(path);

	VM vm;
	initVM(&vm);
	const InterpretResult result = interpret(&vm, source);

	free(source);
	freeVM(&vm);

	if (result != INTERPRET_OK)
		return -1;

	return 0;
}

int main(const int argc, const char* argv[])
{
	if (argc == 1)
		repl();
	else if (argc == 2)
		return runFile(argv[1]);
	else
	{
		printf("Usage: kal [path]\n");
		exit(64);
	}

	return 0;
}