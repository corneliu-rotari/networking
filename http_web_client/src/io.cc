#include "../headers/io.hh"


string promt(string for_what)
{
    string info;
    cout << for_what << "=";
    getline(cin, info);
    return info;
}

json io_get_user_info(void)
{
    json j;
    j["username"] = promt("username");
    j["password"] = promt("password"); //maybe add getpass
    return j;
}

string io_get_id(void)
{
    return promt("id");
}

void io_get_book(void)
{
    // TODO add check for user input
    promt("title");
    promt("author");
    promt("genre");
    promt("publisher");
    promt("page_count");
}

void io_print_error(string content)
{
    cout << "[Error] " << content << endl;
}