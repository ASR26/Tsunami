#include <stdio.h>

int	main (int argc, char **argv)
{
	char buffer[64];
	if (argc < 2)
	{
		printf("Introduzca un argumento al programa\n");
		return 0;
	}
	strcpy (buffer, argv[1]);
	return 0;
}
