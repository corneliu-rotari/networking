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

### Type of messege

**`0` ID** - Packet sent by the user with his id.

**`1` Request** - Packet sent by the user with the request of subscribing and unsubscribing.

**`2` Replay** - Packet sent by the server with the information reqsted by the user.

**`3` Acknowleadgement** - Packet sent by the server for acknowleadge the clinets connection.

### Size and payload

The payload is information that can vary from 0 to 1500 bytes, depening of the type of messege and the information sent by the news clients (udp).

## Server

## Subscriber
