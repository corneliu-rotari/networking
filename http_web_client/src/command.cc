#include "../headers/requests.hh"
#include "../headers/command.hh"
#include "../headers/io.hh"
#include "../headers/network.hh"
#include "../headers/parse_http.hh"

#include <sys/socket.h> /* socket, connect */
#include <stdlib.h>
#include <iostream>

#define _CHECKER_

string send_request(string ip, string access, char *message)
{
    int fd = open_connection((char *)ip.c_str(), 8080, AF_INET, SOCK_STREAM, 0);
    send_to_server(fd, message);
    free(message);

    char *response = receive_from_server(fd);
    string resp_str = string(response);
    close_connection(fd);
    free(response);
    return resp_str;
}

string send_request_with_cookie(string ip, string access, string cookie)
{

    char *message;

    char *access_cookie[1];
    access_cookie[0] = (char *)cookie.c_str();

    message = compute_get_request((char *)ip.c_str(),
                                  (char *)access.c_str(),
                                  NULL, access_cookie, 1);
    return send_request(ip, access, message);
}

string auth_user(string ip, string access_route)
{
    json cred = io_get_user_info();
    if (empty(cred))
    {
        return "";
    }

    string cont_type = "application/json";

    char *message;
    message = compute_post_request((char *)ip.c_str(),
                                   (char *)access_route.c_str(),
                                   (char *)cont_type.c_str(),
                                   (char *)cred.dump().c_str(),
                                   NULL, 0);
    return send_request(ip, access_route, message);
}

void register_user(string ip, string access_route)
{
    string response = auth_user(ip, access_route);
    int http_return_code = get_http_code(response);

    if (check_for_http_errors(response))
    {
        io_print_success("Registered successfully", http_return_code);
    }
}

string login_user(string ip, string access_route)
{
    string response = auth_user(ip, access_route);
    if (response == "")
    {
        io_print_error("Credentials should not contain spaces");
    }

    string to_ret = "";
    int http_return_code = get_http_code(response);

    if (check_for_http_errors(response))
    {
        int cookie_pos = response.find(' ', response.find("Set-Cookie:")) + 1;
        to_ret = response.substr(cookie_pos, response.find("\r\n", cookie_pos) - cookie_pos);

        io_print_success("User was logged in", http_return_code);
    }

    return to_ret;
}

string logout_user(string ip, string access_route, string cookie)
{
    string response = send_request_with_cookie(ip, access_route, cookie);
    if (check_for_http_errors(response))
    {
        io_print_success("You have been logged out", get_http_code(response));
        return "";
    }
    return cookie;
}

string enter_library(string ip, string access_route, string cookie)
{
    string jwt = "", response = send_request_with_cookie(ip, access_route, cookie);
    if (check_for_http_errors(response))
    {
        jwt = get_http_body_json(response)["token"];
        io_print_success("Enterd the library", get_http_code(response));
    }
    return jwt;
}

void get_books(string ip, string access_route, string token)
{
    string acc_bear = "Authorization: Bearer " + token;
    string response = send_request(ip, access_route,
                                   compute_get_request((char *)ip.c_str(),
                                                       (char *)access_route.c_str(),
                                                       (char *)acc_bear.c_str(),
                                                       NULL, 0));
    if (check_for_http_errors(response))
    {
        json book_list = get_http_body_json(response);

#ifdef _CHECKER_
        cout << book_list << endl;
#endif
#ifndef _CHECKER_
        for (json::iterator elem = book_list.begin(); elem != book_list.end(); elem++)
        {
            json book = *elem;
            cout << "[Id] : " << book["id"] << " [Title] : " << book["title"] << endl;
        }
        cout << endl;
#endif
    }
}

void get_book(string ip, string access_route, string token)
{
    string id = io_get_id();
    if (id == "")
    {
        return;
    }
    string new_acc = access_route + "/" + id;
    string acc_bear = "Authorization: Bearer " + token;
    string response = send_request(ip, new_acc,
                                   compute_get_request((char *)ip.c_str(),
                                                       (char *)(new_acc).c_str(),
                                                       (char *)acc_bear.c_str(),
                                                       NULL, 0));
    if (check_for_http_errors(response))
    {
        json book = get_http_body_json(response);

#ifdef _CHECKER_
        cout << book << endl;
#endif
#ifndef _CHECKER_
        cout << "[Id] : " << book["id"] << endl
             << "[Title] : " << book["title"] << endl
             << "[Author] : " << book["author"] << endl
             << "[Publisher] : " << book["publisher"] << endl
             << "[Genre] : " << book["genre"] << endl
             << "[Page_count] : " << book["page_count"] << endl;
#endif
    }
}

void add_book(string ip, string access_route, string token)
{
    json input = io_get_book();
    if (empty(input))
    {
        return;
    }
    string acc_bear = "Authorization: Bearer " + token;
    string cont_type = "application/json";
    char *cookies[1];
    cookies[0] = (char *)acc_bear.c_str();

    string response = send_request(ip, access_route,
                                   compute_post_request((char *)ip.c_str(),
                                                        (char *)access_route.c_str(),
                                                        (char *)cont_type.c_str(),
                                                        (char *)input.dump().c_str(),
                                                        cookies, 1));

    int http_return_code = get_http_code(response);

    if (check_for_http_errors(response) && http_return_code == 200)
    {
        io_print_success("The book was added", get_http_code(response));
    }
}

void delete_book(string ip, string access_route, string token)
{
    string id = io_get_id();
    if (id == "")
    {
        return;
    }
    string new_acc = access_route + "/" + id;
    string acc_bear = "Authorization: Bearer " + token;
    string response = send_request(ip, new_acc,
                                   compute_delete_request((char *)ip.c_str(),
                                                          (char *)(new_acc).c_str(),
                                                          (char *)acc_bear.c_str(),
                                                          NULL, 0));
    int http_return_code = get_http_code(response);

    if (check_for_http_errors(response) && http_return_code == 200)
    {
        io_print_success("The book was added", get_http_code(response));
    }
}
