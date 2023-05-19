# HTTP Client

`Author : Rotari Corneliu` `Group : 323CD`

A http client application that enables communication with a book REST API.
Handles requests and responses with a jwt token.

Added features: `login`, `register`, `logout`, `enter_library`,  `get_books`, `get_book`, `add_book`, `delete_book`, `exit`.

***

## Table of Contents

- [Makefile](#makefile)
- [Project Structure](#project-structure)
- [Functionality](#functionality)
- [Additional Libraries](#additional-libraries-and-code)

***

## Makefile

`make build` - creates the client.

`make clean` - removes the executable.

***

## Project Structure

```tree
http_web_client
├── client.cc           // application entry point
├── headers
│   ├── buffer.hh       // handles buffer
│   ├── command.hh      // handles command execution
│   ├── io.hh           // parses user input and prints user frendly output
│   ├── json.hpp        // nlohmann json parser
│   ├── network.hh      // communication with the server
│   ├── parse_http.hh   // commun http actions
│   └── requests.hh     // creates GET, POST and DELETE request from the client
├── Makefile
├── README.md
└── src                 // implementation for the headers files.
    ├── buffer.cc
    ├── command.cc
    ├── io.cc
    ├── network.cc
    ├── parse_http.cc
    └── requests.cc
```

***

## Functionality

The client recives user commands and parses them, the input validation is handled by the `io.hh` and prints error if there is the case.

Until the program recives `exit` it accepts commands and the only accepted ones are in the description of the project. Prints error if the user enters a differnet command.

The validation of the user authentication and jwt_token presense are handled internaly and there are no request sent if those check are not met.

If the errors are from the server the message contains the error code `parse_http.hh`, on the other hand there are not error code if it occurs in the validation phase.

In [`command.cc`](./src/command.cc) exist a `#define _CHECKER_` that enables output for the checker.
If not defined it prints regular output.

***

## Additional Libraries and Code

[Nlohmann](https://json.nlohmann.me/) - Json parser used for an easier and light weight communication with the server.

[Lab_9](https://gitlab.cs.pub.ro/pcom/pcom-laboratoare-public/-/tree/master/lab9) - Base structre for the project.

***
