
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include <iostream>
#include <string>

#include "headers/network.hh"
#include "headers/requests.hh"
#include "headers/command.hh"
#include "headers/io.hh"

int main(int argc, char *argv[])
{
    string ip = "34.254.242.81";
    string access_route = "/api/v1/tema";

    string user_input = "";
    getline(cin, user_input);

    string login_cookie = "";
    string jwt_token = "";

    while (user_input != "exit")
    {
        if (user_input == "register")
        {
            register_user(ip, access_route + "/auth/register");
        }
        else if (user_input == "login")
        {
            if (login_cookie == "")
            {
                login_cookie = login_user(ip, access_route + "/auth/login");
            }
            else
            {
                io_print_error("You already logged in, please logout to login again");
            }
        }
        else if (user_input == "logout")
        {
            login_cookie = logout_user(ip, access_route + "/auth/logout", login_cookie);
            if (login_cookie == "") {
                jwt_token = "";
            }
        }
        else if (user_input == "enter_library")
        {
            jwt_token = enter_library(ip, access_route + "/library/access", login_cookie);
        }
        else if (user_input == "get_books")
        {
            get_books(ip, access_route + "/library/books", jwt_token);
        }
        else if (user_input == "get_book")
        {
            get_book(ip, access_route + "/library/books", jwt_token);
        }
        else if (user_input == "add_book")
        {
            add_book(ip, access_route + "/library/books", jwt_token);
        }
        else if (user_input == "delete_book")
        {
            delete_book(ip, access_route + "/library/books", jwt_token);
        }
        else
            io_print_error("Invalid command, try again.");

        getline(cin, user_input);
    }

    return 0;
}
