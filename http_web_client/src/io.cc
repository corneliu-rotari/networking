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
    j["password"] = promt("password"); // maybe add getpass
    return j;
}

json io_get_id(void)
{
    json j;
    string id = promt("id");
    if (id.find_first_not_of("0123456789") == std::string::npos)
    {
        j["id"] = id;
        return j;
    }

    io_print_error("Id should contain only numbers");
    return j;
}

json io_get_book(void)
{
    // TODO add check for user input
    json j;
    j["title"] = promt("title");
    j["author"] = promt("author");
    j["genre"] = promt("genre");
    j["publisher"] = promt("publisher");
    j["page_count"] = promt("page_count");
    return j;
}

void io_print_error(string content)
{
    cout << "[Error] " << content << endl
         << endl;
}