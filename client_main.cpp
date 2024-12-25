#include <iostream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h> // Библиотека для HTTP-запросов
#include <nlohmann/json.hpp> // Библиотека для работы с JSON
#include <string>
#include <vector>
#include <exception>

// Структура для хранения информации о пользователе
struct User {
    std::string login;
    unsigned long int id;
    
    User(std::string login, unsigned long int id) : login(login), id(id) {}
};

// Структура для хранения информации о сообщении
struct Message {
    unsigned long int id;
    std::string login_from;
    std::string login_to;
    std::string message;
    Message(unsigned long int id, std::string login_from, std::string login_to, std::string message)
        : id(id), login_from(login_from), login_to(login_to), message(message) {}
};

// Функция для создания пользователя
bool create_user(httplib::Client &cli, const std::string &login, const std::string &password) {
    std::cout << "# request POST /users" << std::endl;

    // Формируем JSON-объект с логином и паролем
    nlohmann::json j_object = {
        {"login", login},
        {"password", password}
    };

    // Отправляем POST-запрос на сервер
    auto res = cli.Post("/users", j_object.dump(4), "application/json");
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            return false;
        }
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
        return false;
    }
    return true;
}

// Функция для получения списка всех пользователей
std::vector<User> get_users(httplib::Client &cli) {
    std::vector<User> result;
    std::cout << "# request GET /users" << std::endl;

    // Отправляем GET-запрос на сервер
    auto res = cli.Get("/users");
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            throw std::logic_error(res->body);
        }
        // Парсим JSON-ответ
        nlohmann::json j_array = nlohmann::json::parse(res->body);

        if (j_array.is_array()) {
            // Итерируемся по массиву пользователей
            for (const auto &item : j_array) {
                result.push_back(User(item["login"], item["id"]));
            }
        }
        return result;
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
        throw std::logic_error(httplib::to_string(res.error()));
    }
}

// Функция для создания сессии (входа пользователя)
unsigned long int create_session(httplib::Client &cli, const std::string &login, const std::string &password) {
    std::cout << "# request POST /session" << std::endl;

    // Формируем JSON-объект с логином и паролем
    nlohmann::json j_object = {
        {"login", login},
        {"password", password}
    };

    // Отправляем POST-запрос на сервер
    auto res = cli.Post("/session", j_object.dump(4), "application/json");
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            return 0;
        }

        // Парсим JSON-ответ и возвращаем ID сессии
        nlohmann::json j = nlohmann::json::parse(res->body);
        return j["Session Info"]["session id"];
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
        return 0;
    }
}

// Функция для отправки сообщения
bool send_message(httplib::Client &cli, unsigned long int session_id, const std::string &login_to, const std::string &message) {
    std::cout << "# request POST /messages" << std::endl;

    // Формируем JSON-объект с ID сессии, получателем и текстом сообщения
    nlohmann::json j_object = {
        {"session", session_id},
        {"to", login_to},
        {"message", message}
    };

    // Отправляем POST-запрос на сервер
    auto res = cli.Post("/messages", j_object.dump(4), "application/json");
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            return false;
        }
        return true;
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
    }

    return false;
}

// Функция для отправки отложенного сообщения
bool send_delayed_message(httplib::Client &cli, unsigned long int session_id, const std::string &login_to, const std::string &message) {
    std::cout << "# request POST /delayed_messages" << std::endl;

    // Формируем JSON-объект с ID сессии, получателем и текстом сообщения
    nlohmann::json j_object = {
        {"session", session_id},
        {"to", login_to},
        {"message", message}
    };

    // Отправляем POST-запрос на сервер
    auto res = cli.Post("/delayed_messages", j_object.dump(4), "application/json");
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            return false;
        }
        return true;
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
    }

    return false;
}

// Функция для получения сообщений
std::vector<Message> get_messages(httplib::Client &cli, unsigned long int session_id, bool all) {
    std::vector<Message> result;

    std::cout << "# request GET /messages" << std::endl;
    // Формируем строку запроса с параметрами
    std::string query = "/messages?session=" + std::to_string(session_id) + "&all=";
    query += (all ? "true" : "false");

    std::cout << "# request GET " << query << std::endl;
    // Отправляем GET-запрос на сервер
    auto res = cli.Get(query);
    if (res) {
        std::cout << "Result code:" << res->status << std::endl;
        if (res->status != 200) {
            std::cout << "Result body:" << res->body << std::endl;
            throw std::logic_error(res->body);
        }
        // Парсим JSON-ответ
        nlohmann::json j_array = nlohmann::json::parse(res->body);

        if (j_array.is_array()) {
            // Итерируемся по массиву сообщений
            for (const auto &item : j_array) {
                result.push_back(Message(item["id"], item["login_from"], item["login_to"], item["message"]));
            }
        }
        return result;
    } else {
        std::cout << "error: " << httplib::to_string(res.error()) << std::endl;
        throw std::logic_error(httplib::to_string(res.error()));
    }
}

// Основная функция
auto main() -> int {
    httplib::Client cli("localhost", 8080); // Создаем клиент для подключения к серверу

    // Создаем двух пользователей
    create_user(cli, "Jhon Doe", "qwerty");
    create_user(cli, "Jane Doe", "12345678");

    // Выводим список всех пользователей
    std::cout << "Users:" << std::endl;
    for (auto u : get_users(cli)) {
        std::cout << u.id << "," << u.login << std::endl;
    }

    std::cout << std::endl;

    // Создаем сессию для первого пользователя и отправляем отложенное сообщение
    int session_sender = create_session(cli, "Jhon Doe", "qwerty");
    if (session_sender > 0) {
        std::cout << "Create session:" << session_sender << std::endl;
        send_delayed_message(cli, session_sender, "Jane Doe", "This is a delayed message!");
    }

    // Создаем сессию для второго пользователя и получаем сообщения
    std::cout << "Start to read messages " << std::endl;
    int session_reader = create_session(cli, "Jane Doe", "12345678");
    if (session_reader > 0) {
        std::cout << "Messages:" << std::endl;
        for (auto m : get_messages(cli, session_reader, true)) {
            std::cout << "[" << m.id << "] from=" << m.login_from << " message=" << m.message << std::endl;
        }
    }

    return 0;
}