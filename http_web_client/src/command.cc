#include "../headers/requests.hh"
#include "../headers/command.hh"
#include "../headers/io.hh"
#include "../headers/network.hh"
#include "../headers/parse_http.hh"

#include <sys/socket.h> /* socket, connect */
#include <stdlib.h>

char *auth_user(string ip, string access_route)
{
    int fd = open_connection((char *)ip.c_str(), 8080, AF_INET, SOCK_STREAM, 0);

    json cred = io_get_user_info();
    string cont_type = "application/json";

    char *message, *response;
    message = compute_post_request((char *)ip.c_str(),
                                   (char *)access_route.c_str(),
                                   (char *)cont_type.c_str(),
                                   (char *)cred.dump().c_str(),
                                   NULL, 0);
    send_to_server(fd, message);
    free(message);

    response = receive_from_server(fd);
    close_connection(fd);
    return response;
}

void register_user(string ip, string access_route)
{
    char *response = auth_user(ip, access_route);

    string resp_str = string(response);
    int http_return_code = get_http_code(resp_str);

    if (is_http_error(http_return_code))
    {
        json error = get_http_error_msg(resp_str);
        io_print_error(error["error"]);
        free(response);
        return;
    }

    if (http_return_code == 201)
    {
        cout << "Registered successfully\n"
             << endl;
    }

    free(response);
}

string login_user(string ip, string access_route)
{
    char *response = auth_user(ip, access_route);

    string resp_str = string(response);
    string to_ret = "";
    int http_return_code = get_http_code(resp_str);

    if (is_http_error(http_return_code))
    {
        json error = get_http_error_msg(resp_str);
        io_print_error(error["error"]);
    }
    else if (http_return_code == 200)
    {
        int cookie_pos = resp_str.find(' ', resp_str.find("Set-Cookie:")) + 1;
        to_ret = resp_str.substr(cookie_pos, resp_str.find("\r\n", cookie_pos) - cookie_pos);

        cout << "The User was logged in\n"
             << endl;
    }

    free(response);
    return to_ret;
}

void logout_user(string ip, string access_route)
{
}

void enter_library(int fd, string ip, string access_route)
{
}

void get_books(int fd, string ip, string access_route)
{
}

void get_book(int fd, string ip, string access_route)
{
    json id = io_get_id();
    if (empty(id))
    {
        return;
    }
}

void add_book(int fd, string ip, string access_route)
{
}

void delete_book(int fd, string ip, string access_route)
{
    json id = io_get_id();
    if (empty(id))
    {
        return;
    }
}
