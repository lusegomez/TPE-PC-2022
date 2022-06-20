# PROXY-SOCKS5 - PROTOCOLOS DE COMUNICACION - 2022-1C

## Tabla de contenidos

[**Descripción del proyecto**](#descripcion)

[**Dependencias**](#dependencias)

[**Instalación**](#instalacion)

[**Instrucciones para el uso**](#instrucciones)

 * [**Servidor**](#server)
 
 * [**Cliente**](#client)
  
[**Documentación del servidor de monitoreo**](#mng-doc)

<a name="descripcion"></a>
## Descripción del proyecto
El objetivo del trabajo es implementar un servidor proxy para el protocolo SOCKSv5[RFC1928], el cual puede atender multiples clientes en forma concurrente y simultanea.

<a name="dependencias"></a>
## Dependencias
El proyecto requiere, como unica dependencia, la libreria libsctp-dev para la utilizacion del protocolo SCTP. Para distribuciones basadas en debian ejecutar el siguiente comando:
```bash
sudo apt install libsctp-dev
```

<a name="instalacion"></a>
## Instalación
El proyecto se compila utilizando el siguiente comando en el directorio src del proyecto:
```bash
make all
```

Además, se puede especificar el compilador a utilizar, por ejemplo:
```bash
CC=gcc make all 
```

Esto genera todos los archivos binarios necesarios para ejecutarlo.
Si se desea eliminar los binarios se debe ejecutar el siguiente comando en el directorio src del proyecto:
```bash
make clean
```

<a name="instrucciones"></a>
## Instrucciones para el uso
<a name="server"></a>
### Servidor
Para ejecutar el servidor se debe ejecutar el siguiente comando:
```bash
./socks5d <options>
```
Donde options puede ser una o mas opciones de las siguientes:
```bash
OPCIONES
       -h     Imprime la ayuda y termina.

       -l dirección-socks
              Establece la dirección donde servirá el proxy SOCKS.  Por defecto escucha en todas las interfaces.

       -N     Deshabilita los passwords disectors.

       -L dirección-de-management
              Establece la dirección donde servirá el servicio de management. Por defecto escucha únicamente en loopback.

       -p puerto-local
              Puerto TCP donde escuchará por conexiones entrantes SOCKS.  Por defecto el valor es 1080.

       -P puerto-conf
              Puerto SCTP  donde escuchará por conexiones entrante del protocolo de configuración. Por defecto el valor es 8080.

       -u user:pass
              Declara un usuario del proxy con su contraseña. Se puede utilizar hasta 10 veces.

       -v     Imprime información sobre la versión versión y termina.
```
Las distintas opciones estan especificadas en su manual (socks5d.8). Para acceder a él, se debe ejecutar el siguiente comando:
```bash
man ./socks5d.8
```

### Cliente
Y luego ejecutar el siguiente comando:
```bash
./socks5dctl <options>
```
Donde options puede ser una o mas opciones de las siguientes:
```bash
OPCIONES
       -L dirección-de-management
                Establece la dirección donde sirve el servicio de
                management. Por defecto usa localhost.

       -P puerto-conf
                Puerto SCTP  donde escucha el protocolo
                de configuración. Por defecto el valor es 8080.

       -h
                Imprime la ayuda y termina.
```
Las distintas opciones estan especificadas en su manual (socks5dctl.8). Para acceder a él, se debe ejecutar el siguiente comando:
```bash
man ./socks5dctl.8
```

<a name="mng-doc"></a>
## Documentación del servidor de monitoreo
La documentacion del servidor de monitoreo se encuentra ubicada en el archivo "Protocolo de monitoreo.pdf"
