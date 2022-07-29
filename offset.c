#include <stdio.h>
#include <windows.h>
typedef VOID (*MYPROC)(LPTSTR);

int main (int argc, char **argv)
{
	char dll[100];
	char function[100];

	HINSTANCE libreria;
	MYPROC procadd;

	printf("Busca offsets. Inrtoduce como primer argumento el nombre de la DLL\n");
	printf("y como segundo argumento la función dentro de la DLL\n");
	print("Por ejemplo %s msvcrt.dll system\n\n", argv[0]);

	if (argc != 3)
	{
		printf ("Introduce 2 argumentos\n");
		return (1);
	}
	memset(dll, 0, sizeof(dll));
	memset(funcion, argv[2], strlen(argv[2]));

	libreria = LoadLibrary(dll);
	procadd = (MYPROC)GetProcAdress (libreria, fucnion);

	printf("Offset de %s en la DLL %s es %x", funcion, dll, proadd);

	return (0);
}
