#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Compiler.h"
#include "instructions.h"
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

	// Add a newline at end of file in case there wasn't one already, otherwise our scanner will get upset.
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

static int outputBytecode(const char* sourcePath, const char* outputPath)
{
	Bytecode bytecode;
	initBytecode(&bytecode);

	size_t jumpTable[256];
	uint8_t ram[256];

	char* source = readFile(sourcePath);

	const bool compileSuccess = compile(&bytecode, jumpTable, ram, source);

	if (!compileSuccess)
	{
		freeBytecode(&bytecode);
		free(source);
		return -1;

	}

	FILE* outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		printf("Could not open file '%s'.\n", outputPath);
		return 74;
	}

	size_t writeSuccess = fwrite(jumpTable, sizeof(jumpTable), 1, outputFile);
	if (!writeSuccess)
	{
		printf("Error while writing to output file.\n");
		return -1;
	}

	writeSuccess = fwrite(ram, sizeof(ram), 1, outputFile);
	if (!writeSuccess)
	{
		printf("Error while writing to output file.\n");
		return -1;
	}

	writeSuccess = fwrite(bytecode.code, bytecode.count * sizeof(uint8_t), 1, outputFile);
	if (!writeSuccess)
	{
		printf("Error while writing to output file.\n");
		return -1;
	}

	fclose(outputFile);

	printf("Success. Wrote %llu byte bytecode file.",
		sizeof(jumpTable) + sizeof(ram) + bytecode.count * sizeof(uint8_t));

	freeBytecode(&bytecode);
	free(source);
	return 0;
}

static int runBytecode(const char* path)
{
	void* fileData = readFile(path);
	VM vm;
	initVM(&vm);

	const size_t* jumpTable = fileData;
	const uint8_t* ram = fileData + 256 * sizeof(size_t);

	for (int i = 0; i < 256; ++i)
		vm.jumpTable[i] = jumpTable[i];
	for (int i = 0; i < 256; ++i)
		vm.ram[i] = ram[i];

	vm.bytecode.code = fileData + 256 * sizeof(size_t) + 256 * sizeof(uint8_t);
	vm.ip = vm.bytecode.code;
	const InterpretResult result = run(&vm);

	free(fileData);
	// Do NOT free VM as it will just try to free the bytecode pointer.

	if (result != INTERPRET_OK)
		return -1;

	return 0;
}

static int invalidUsage()
{
	printf("Usage: kal [path] [flags]\n");
	return 64;
}

int main(const int argc, const char* argv[])
{
	if (argc == 1)
		repl();
	else if (argc == 2)
	{
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
		{
			printf("Usage help:\n"
				"Run the repl: kal\n"
				"Run a source file: kal <file-path>\n"
				"Compile a source file and output the bytecode to a file: kal <source-file> -o <output-file>\n"
				"Run a binary file containing kal bytecode: kal <file-path> -b\n");
			return 0;
		}

		return runFile(argv[1]);
	}
	else if (argc == 3)
	{
		if (strcmp(argv[2], "-b") == 0) // Run bytecode file.
			return runBytecode(argv[1]);

		return invalidUsage();
	}
	else if (argc == 4)
	{
		if (strcmp(argv[2], "-o") == 0) // Output bytecode
			return outputBytecode(argv[1], argv[3]);

		return invalidUsage();
	}
	else
		return invalidUsage();

	return 0;
}