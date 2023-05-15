#ifndef _COMMANDS_
#define _COMMANDS_
#include <string>
#include <iostream>

using namespace std;

void login_user(string ip, string access_route);
void register_user(string ip, string access_route);

void logout_user(int fd, string ip, string access_route);
void enter_library(int fd, string ip, string access_route);
void get_books(int fd, string ip, string access_route);
void get_book(int fd, string ip, string access_route);
void add_book(int fd, string ip, string access_route);
void delete_book(int fd, string ip, string access_route);

#endif