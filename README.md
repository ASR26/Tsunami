<h1>Descripción</h1>
<p>El proyecto Tsunami consiste en hacer un programa vulnerable a <a href="https://es.wikipedia.org/wiki/Desbordamiento_de_b%C3%BAfer">buffer overflow</a>, y otro que aproveche esta vulnerabilidad para ejecutar la calculadora</p>
<p>Para este proyecto se usa un <a href="https://javiermartinalonso.github.io/devops/devops/vagrant/2018/02/09/vagrant-vagrantfile.html">Vagrantfile</a> que emulará una máquina Windows XP 32 bits donde se hará todo el proceso</p>
<h2>Archivos</h2>
<p>A continuación se hará una breve descripción de cada archivo y su función. El funcionamiento interno de cada archivo se encuentra explicado con más detalle en los comentarios de cada uno.</p>
<h3>Vuln1.c</h3>
<p>Este archivo es el ejecutable vulnerable al buffer overflow, debido a que usa la función strcpy y no comprueba la longitud que de los datos introducidos en el buffer</p>
<h3>Offset.c</h3>
<p>Esta función se usa en el proceso de creación del exploit. Al darle como argumentos una librería y una función dentro de la misma nos devolverá una dirección de memoria, donde se encuentra la función en la máquina (más adelante veremos su uso)</p>
<h3>Shellcode.c</h3>
<p>Este archivo contiene el código que iniciará la calculadora. El código está en ensamblador (se pueden encontrar ejemplos similares en internet) y para obtener las dos direcciones de memoria que podemos ver dentro del código se ha usado el offset.c</p>
<h3>Findjmp.exe</h3>
<p>Este archivo se usa para conseguir el offset de la librería kernel32.dll que necesitaremos para el shellcode</p>
<h3>Exploit.c</h3>
<p>Este es el programa que aprovechará la vulnerabilidad del programa vuln1 para desbordar el buffer y abrir la calculadora</p>

<h2>Funcionamiento</h2>
<p>El primer paso será compilar el archivo vulnerable de la siguiente forma:</p>

```
cl vuln1.c
```

Esto nos dará un archivo .obj y un archivo .exe, que será el ejecutable vulnerable.<br>
El siguiente paso será buscar los offsets necesarios para crear el shellcode, para ello compilaremos el archivo offset.c y lo ejecutaremos de la siguiente forma:

```
cl offset.c
offset.exe kernel32.dll LoadLibraryA 
offset.exe msvcrt.dll system
```

El comando cl nos dará el archivo .exe que será el que usaremos para los siguientes comandos, ambos nos darán como resultado las siguientes direcciones de memoria:<br>
`0x7c801d7b`<br>
`0x77c293c7`<br>

Estás direcciones son las correspondientes a las funciones (tercer argumento de los comandos) en las librerías (segundo argumento en los comandos). Necesitaremos esta dirección para ejecutar las funciones desde lenguaje ensamblador.

Ahora haremos el shellcode con las direcciones que hemos conseguido
