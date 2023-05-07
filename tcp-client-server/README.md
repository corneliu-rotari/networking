# News Letter TCP Application

`Author : Rotari Corneliu` `Group : 323CD`

***

## Table of contents

- [Protocol](#application-protocol)
- [Server](#server)
- [Subscriber](#subscriber)

***

## Application Protocol

```protocol
0                   7                  23
+-------------------+-------------------+
|  Type of Messege  |  Size of payload  |
+-------------------+-------------------+
|     Depends on the type of messege    |
+----------------------------------------
```

The messeges are sent at first the type and size and after that the actual payload.
Everything is in a union to easly cast in the programm.

### Type of messege

**`0` ID** - Packet sent by the user with his id.

**`1` Request** - Packet sent by the user with the request of subscribing and unsubscribing.

**`2` Replay** - Packet sent by the server with the information reqsted by the user.

**`3` Acknowleadgement** - Packet sent by the server for acknowleadge the clinets connection.

### Size and payload

The payload is information that can vary from 0 to 1500 bytes, depening of the type of messege and the information sent by the news clients (udp).

## Server

The server is using 2 static sockets for tcp listening and udp reciving.
Multiplexing is being created using `poll.h`.
The memory is stored into data structre where everything is connected.
The topic is created only when a tcp subscriber has subscribed.
All the topics are ignored if none of the subscribers are waiting for a response.
The servers prints if the clients connect or to disconnect.

## Subscriber

At first the client connects and sends it's id to the server, and awaits and ack response.
Parses the input and sends the request to the server and awaits for a response.
The multiplexing part is implemented with `poll.h`, for mannaging input and server messages.
Every type of messages are parsed by casting.

## Common

`DIE` - Macro for checking errors and exiting with failure.
`recv`, `send` and the application structre are adapted from laboratory.
