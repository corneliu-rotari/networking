#include "../headers/parse_http.hh"

int get_http_code(string response)
{
    int code_pos = response.find(' ') + 1;
    string code = response.substr(code_pos, response.find(" ", code_pos));
    return stoi(code);
}

bool is_http_error(int code)
{
    return code >= 400;
}

json get_http_body_json(string response)
{
    json msg = json::parse(response.substr(response.find("\r\n\r\n") + 4));
    return msg;
}

bool check_for_http_errors(string response)
{
    int http_return_code = get_http_code(response);
    bool is_error = is_http_error(http_return_code);
    if (is_error)
    {
        json error = get_http_body_json(response);
        io_print_error(error["error"], http_return_code);
    }
    return !is_error;
}
