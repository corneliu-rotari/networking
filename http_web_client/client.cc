
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
    int sockfd;
    string ip = "34.254.242.81";
    string access_route = "/api/v1/tema";

    string user_input = "";
    getline(cin, user_input);

    while (user_input != "exit")
    {

        if (user_input == "register")
        {
            register_user(ip, access_route + "/auth/register");
        }
        else if (user_input == "login")
        {
            login_user(ip, access_route + "/auth/login");
        }
        else if (user_input == "logout")
        {
            logout_user(sockfd, ip, access_route + "/auth/logout");
        }
        else if (user_input == "enter_library")
        {
            enter_library(sockfd, ip, access_route + "/library/access");
        }
        else if (user_input == "get_books")
        {
            get_books(sockfd, ip, access_route + "/library/books");
        }
        else if (user_input == "get_book")
        {
            get_book(sockfd, ip, access_route + "/library/books");
        }
        else if (user_input == "add_book")
        {
            add_book(sockfd, ip, access_route + "/library/books");
        }
        else if (user_input == "delete_book")
        {
            delete_book(sockfd, ip, access_route + "/library/books");
        }
        else
            io_print_error("Invalid command, try again.");

        getline(cin, user_input);
    }

    return 0;
}
