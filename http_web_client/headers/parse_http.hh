#ifndef _HTTP_PARSE_
#define _HTTP_PARSE_
#include "../headers/io.hh"

int get_http_code(string response);
bool is_http_error(int code);
json get_http_body_json(string respone);
bool check_for_http_errors(string response);

#endif
