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

json get_http_error_msg(string response) {
    json msg = json::parse(response.substr(response.find("{\"")));
    return msg;
}
