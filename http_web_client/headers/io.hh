#ifndef _INPUT_PARSER_
#define _INPUT_PARSER_

#include <string>
#include "../headers/json.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace std;

// Input

json io_get_user_info(void);
string io_get_id(void);
json io_get_book(void);

// Output

void io_print_error(string content, int code = -1);
void io_print_success(string content, int code = -1);

#endif
