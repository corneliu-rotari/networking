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

bool is_only_digits(string str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool has_a_digit(string str)
{
    return str.find_first_of("0123456789") != std::string::npos;
}

string io_get_id(void)
{
    string id = promt("id");
    if (is_only_digits(id))
    {
        return id;
    }
    io_print_error("Id should contain only numbers");
    return "";
}

json io_get_book(void)
{
    json j;
    string title = promt("title");
    string author = promt("author");
    if (has_a_digit(author))
    {
        io_print_error("Author name should not contain numbers");
        return j;
    }
    string genre = promt("genre");

    if (has_a_digit(genre))
    {
        io_print_error("Genre should not contain numbers");
        return j;
    }
    string publisher = promt("publisher");
    string page_count = promt("page_count");

    if (!is_only_digits(page_count))
    {
        io_print_error("Page count should contain only numbers");
        return j;
    }
    j["title"] = title;
    j["author"] = author;
    j["genre"] = genre;
    j["publisher"] = publisher;
    j["page_count"] = stoi(page_count);
    return j;
}

void io_print_success(string content, int code)
{
    cout << "[Success";

    if (code != -1)
    {
        cout << " " << code;
    }
    
    cout << "] : " << content << endl
         << endl;
}

void io_print_error(string content, int code)
{
    cout << "[Error";

    if (code != -1)
    {
        cout << " " << code;
    }

    cout << "] : " << content << endl
         << endl;
}
