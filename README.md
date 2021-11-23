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
