# PROXY-SOCKS5 - PROTOCOLOS DE COMUNICACION - 2022-1C

## Tabla de contenidos
1. [**Descripción del proyecto**] (#descripcion)
2. [**Instalación**] (#instalacion)
3. [**Instrucciones para el uso**] (#instrucciones)
  * [**Servidor**] (#server)
  * [**Cliente**] (#client)
4. [**Documentación del servidor de monitoreo**] (#mng-doc)

## Descripción del proyecto
El objetivo del trabajo es implementar un servidor proxy para el protocolo SOCKSv5[RFC1928], el cual puede atender multiples clientes en forma concurrente y simultanea.

## Instalación
El proyecto se compila utilizando el siguiente comando en el directorio src del proyecto:
```bash
make all
```

Además, se puede especificar el compilador a utilizar, por ejemplo:
```bash
make cc=gcc all 
```

Esto genera todos los archivos binarios necesarios para ejecutarlo.
Si se desea eliminar los binarios se debe ejecutar el siguiente comando en el directorio src del proyecto:
```bash
make clean
```

## Instrucciones para el uso
### Servidor
Para ejecutar el servidor se debe ejecutar el siguiente comando:
```bash
./socks5d <options>
```
Las distintas opciones estan especificadas en su manual (socks5d.8). Para acceder a él, se debe ejecutar el siguiente comando:
```bash
man ./socks5d.8
```

### Cliente
