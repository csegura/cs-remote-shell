# Sample Remote Shell using sockets 

## Motivaltion

These are sample programs that I have written to learn how linux sockets run. Why? because I had some free time, because I always have wanted understand it, because I am fivetyone years and code anything relax me. I don't know. I do a lot of crazy things.

Follow me on TWITCH - https://www.twitch.tv/romheat LIVE CODE

## Server

The server program, is a remote shell any command sent to this is executed and the output is sent back to the client. The server by defaults runs on port 8080 another ports maybe used using command line.

```s
server -p <port>
```

Each accepted connection run in a child process. You can end the server with SIGINT (^C) each accepted connection is showed in the console with the @pid that manage this and the IP for the remote client. When a command is received it is also showed, after each execution the time taken and the bytes sent to client are also showed.

```js
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

```console
client [-s <server>] [-p <port>]
```

```js
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

``` js
./b-server -s 192.168.1.226
Server started.
Socket successfully binded at port 8080.
[  1:  4] Connection accepted from 192.168.1.226
[  2:  5] Connection accepted from 192.168.1.226
[  3:  6] Connection accepted from 192.168.1.226
(  5:  1) REQU: 104857600 - 100.00 MB 
[  4:  7] Connection accepted from 192.168.1.226
(  4:  0) REQU: 104857600 - 100.00 MB 
(  7:  3) REQU: 104857600 - 100.00 MB 
(  6:  2) REQU: 104857600 - 100.00 MB 
[  5:  9] Connection accepted from 192.168.1.226
[  6: 13] Connection accepted from 192.168.1.226
[  7: 14] Connection accepted from 192.168.1.226
[  8: 15] Connection accepted from 192.168.1.226
[  9: 16] Connection accepted from 192.168.1.226
[ 10: 17] Connection accepted from 192.168.1.226
( 15:  7) REQU: 104857600 - 100.00 MB 
( 14:  6) REQU: 104857600 - 100.00 MB 
( 17:  9) REQU: 104857600 - 100.00 MB 
( 13:  5) REQU: 104857600 - 100.00 MB 
( 16:  8) REQU: 104857600 - 100.00 MB 
(  9:  4) REQU: 104857600 - 100.00 MB 
( 16:  8) SENT: 100.00 MB [50185251 hash] in 418.22ms - 239.11 Mb/s
(  4:  0) SENT: 100.00 MB [93042670 hash] in 420.27ms - 237.94 Mb/s
( 17:  9) SENT: 100.00 MB [61529317 hash] in 521.24ms - 191.85 Mb/s
(  5:  1) SENT: 100.00 MB [15586276 hash] in 781.74ms - 127.92 Mb/s
(  9:  4) SENT: 100.00 MB [89306644 hash] in 971.53ms - 102.93 Mb/s
(  6:  2) SENT: 100.00 MB [8453611 hash] in 51.22ms - 95.13 Mb/s
( 13:  5) SENT: 100.00 MB [3628240 hash] in 979.35ms - 102.11 Mb/s
( 15:  7) SENT: 100.00 MB [60276607 hash] in 67.25ms - 93.70 Mb/s
(  7:  3) SENT: 100.00 MB [56360598 hash] in 46.68ms - 95.54 Mb/s
( 14:  6) SENT: 100.00 MB [61254008 hash] in 31.72ms - 96.93 Mb/s
```

``` s
b-client usage
 -s <server> 
 -p <port> 
 -c <size in kilobytes> 
 -f <number of threads>
```

``` js
 ./b-client -s 192.168.1.226 -p 8080 -c 102400 -f 10
n_requests: 10
[11:  8] -OK- RCVD: Message 100.00 MB in 5.8860s - 16.99 Mb/s
[ 3:  0] -OK- RCVD: Message 100.00 MB in 6.4126s - 15.59 Mb/s
[12:  9] -OK- RCVD: Message 100.00 MB in 7.2137s - 13.86 Mb/s
[ 4:  1] -OK- RCVD: Message 100.00 MB in 7.7900s - 12.84 Mb/s
[ 7:  4] -OK- RCVD: Message 100.00 MB in 7.8502s - 12.74 Mb/s
[ 5:  2] -OK- RCVD: Message 100.00 MB in 8.0498s - 12.42 Mb/s
[ 8:  5] -OK- RCVD: Message 100.00 MB in 8.1170s - 12.32 Mb/s
[10:  7] -OK- RCVD: Message 100.00 MB in 8.1473s - 12.27 Mb/s
[ 6:  3] -OK- RCVD: Message 100.00 MB in 8.1599s - 12.26 Mb/s
[ 9:  6] -OK- RCVD: Message 100.00 MB in 8.1759s - 12.23 Mb/s
Elapsed time 8.187156s - 0 errors - 1000.00 MB received
Speed 122.14 Mb/s
```


NOTE2: at least twenty years without writing a line in C 

### Maintainers

[csegura](https://github.com/csegura).

You can follow me on twitter: [![Twitter](http://i.imgur.com/wWzX9uB.png)@romheat](https://www.twitter.com/romheat)

and Twitch -> https://www.twitch.tv/romheat