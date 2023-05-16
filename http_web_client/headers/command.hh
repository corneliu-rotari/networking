#ifndef _COMMANDS_
#define _COMMANDS_
#include <string>
#include <iosfwd>

using namespace std;

string login_user(string ip, string access_route);
void register_user(string ip, string access_route);
string logout_user(string ip, string access_route, string cookie);
string enter_library(string ip, string access_route, string cookie);
void get_books(string ip, string access_route, string token);
void get_book(string ip, string access_route, string token);
void add_book(string ip, string access_route, string token);
void delete_book(string ip, string access_route, string token);

#endif