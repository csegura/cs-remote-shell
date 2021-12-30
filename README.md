# Sample Remote Shell using sockets 

## Motivaltion

These are sample programs that I have written to learn how linux sockets run. Why? because I had some free time, because I always have wanted understand it, because I am fivetyone years and code anything relax me. I don't know. I do a lot of crazy things.

## Server

The server program, is a remote shell any command sent to this is executed and the output is sent back to the client. The server by defaults runs on port 8080 another ports maybe used using command line.

```s
server -p <port>
```

Each accepted connection run in a child process. You can end the server with SIGINT (^C) each accepted connection is showed in the console with the @pid that manage this and the IP for the remote client. When a command is received it is also showed, after each execution the time taken and the bytes sent to client are also showed.

```s
Socket successfully created..
Socket successfully binded at port 8080.
Server listening..
(@390002) Connection accepted from 127.0.0.1

(@390038) RCMD: ls -la
RESP: 0.000000ms - 279 bytes
total 36
drwxr-xr-x 2 romheat romheat  4096 dic 29 22:26 .
drwxr-xr-x 7 romheat romheat  4096 dic 29 22:30 ..
-rw-r--r-- 1 romheat romheat    44 dic 28 19:23 Makefile
-rwxr-xr-x 1 romheat romheat 18424 dic 29 22:26 server
-rw-r--r-- 1 romheat romheat  3954 dic 29 22:30 server.c
```

## Client 

The client is a very basic tcp client that gets a command from the console, send it to the server and wait to show the reply.

The client takes two optional parameters server and port, by defualt server is localhost and port 8080

```sh
client [-s <server>] [-p <port>]
```

```sh
./client -s 127.0.0.1 -p 8080
Connecting to 127.0.0.1:8080
Socket successfully created..
Connected.

CMD: ls -la

RESP: 279 bytes
total 36
drwxr-xr-x 2 romheat romheat  4096 dic 29 22:26 .
drwxr-xr-x 7 romheat romheat  4096 dic 29 22:30 ..
-rw-r--r-- 1 romheat romheat    44 dic 28 19:23 Makefile
-rwxr-xr-x 1 romheat romheat 18424 dic 29 22:26 server
-rw-r--r-- 1 romheat romheat  3954 dic 29 22:30 server.c

CMD: 
```

The conection remains alive between commands, in order to close the client conection type ```exit``` 

NOTE: Take care with the commands sent to server.

## b-server & b-client

It's the same but server send a lot of random bytes to the client, the server read from client command only a number that will be the size in kilobytes that will be sent from the server. The message has a hash that is checked in the client part I want use it to send some kind of codified chunks of data one day and also to test different data compression methods. Do you remember Silicon Valley tv show?, it could be the begining of a new "Pied Pipper" 

NOTE2: at least twenty years without writing a line in C 

### Maintainers

[csegura](https://github.com/csegura).

You can follow me on twitter: [![Twitter](http://i.imgur.com/wWzX9uB.png)@romheat](https://www.twitter.com/romheat)