# Protocolos de Comunicación TPE2

## Autores

- [Julián Arce](https://github.com/juarce)
- [Roberto Catalán](https://github.com/rcatalan98)
- [Paula Domingues](https://github.com/pdomins)
- [Gian Luca Pecile](https://github.com/glpecile)

## Objetivo

El objetivo del TPE2 es la implementación de un proxy que hace uso del protocolo pop3, a su vez el diseño de un protocolo propio para usauarios administrativos.


## Compilación

La compilación se realiza con el siguiente comando:

```bash
make all
```

## Ejecución

En el mismo directorio, luego de la compilación y junto al `informe_tpe2`, 
se encuentran los archivos ejecutables:

* `pop3filter`
* `pop3ctl`

Para ejecutar los mismos:
```bash
./pop3filter <comando> [Origin Server]
```
Las opciones disponibles para ingresar son las siguientes:

| Comandos |  Descripcion | 
|----| -------------------------------------------------------------------------------------------------------------------------- |
| -e | Especifica el archivo donde se redirecciona stderr de las ejecuciones de los filtros. Por defecto el archivo es /dev/null. |
| -h | Imprime la ayuda y termina. |
| -l | Establece la dirección donde servirá el proxy.  Por defecto  escucha en todas las interfaces. |
| -L | Establece  la dirección donde servirá el servicio de management. Por defecto escucha únicamente en loopback. |
| -o | Puerto donde se encuentra el servidor de  management. Por  defecto el valor es 9090. |
| -p | Puerto  TCP  donde escuchará por conexiones entrantes POP3.  Por defecto el valor es 1110. |
| -P | Puerto TCP donde se encuentra el servidor POP3  en  el  servidor origen.  Por defecto el valor es 110. |
| -t | Utilizado para las transformaciones externas.  Compatible con system(3).  La sección FILTROS describe como es la  interacción entre pop3filter y el comando filtro. Por defecto no se aplica ninguna transformación. |
| -v | Imprime información sobre la versión versión y termina. |


```bash
./pop3ctl
```
La contraseña por defecto del usuario admin es `000000`. Dentro del mismo se deben correr de la siguiente manera:
```bash
000000 <comando>
```
Los comandos son **CASE SENSITIVE** y se encuentran listados en la siguiente tabla:

| Comandos |  Descripcion |
|----| ------------------ |
| GET_BUFF_SIZE | Imprime el tamaño del buffer actual. |
| GET_STATS     | Imprime primero las conexiones históricas, luego las conexiones concurrentes y por último la cantidad de bytes transferidos. |
| SET_AUTH      | Cambiar la contraseña del administrador, la misma debe constar de 6 caracteres. |
| HELP          | Listar todos los comandos que tiene a disposición el cliente. |

## Testeo
Para el testeo tanto con **Cppcheck** como **Valgrind**. Correr el siguiente comando:

```bash
 make test
```

## Limpieza

La limpieza de los archivos generados se realiza con el siguiente comando:

```bash
make clean
```
La limpieza de los achivos generados durante el testeo se realiza con el siguiente comando:

```bash
make cleanTest
```
