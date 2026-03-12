#include <stdio.h>
#include <stdlib.h>

#include "Scanner.h"

static void repl()
{
	printf(" _  __   _   _     \n");
	printf("| |/ /  /_\\ | |   \n");
	printf("| ' <  / _ \\| |__  K Assembly Language\n");
	printf("|_|\\_\\/_/ \\_\\____| v0.1.0\n");
	printf("\nPress return without typing anything to exit.\n\n");

	char line[1024];

	for (;;)
	{
		printf("> ");

		// Exit if fgets fails or user presses enter without typing anything (only a newline in line).
		if (!fgets(line, sizeof(line), stdin) || line[1] == '\0')
		{
			printf("\n");
			break;
		}

		Scanner scanner;
		initScanner(&scanner, line);
		tokenize(&scanner);
		for (int i = 0; i < scanner.tokenArray.count; ++i)
			printf("%d ", scanner.tokenArray.tokens[i].type);
		freeScanner(&scanner);
	}
}

static void runFile(const char* path)
{

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