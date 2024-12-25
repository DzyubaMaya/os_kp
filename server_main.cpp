#include <iostream>
#include <cstdlib>
#include <httplib.h> // Библиотека для HTTP-сервера
#include <nlohmann/json.hpp> // Библиотека для работы с JSON
#include <string>
#include <sstream>
#include <map>
#include <vector>

using json = nlohmann::json;

// Структура для хранения информации о сообщении
struct Message {
    unsigned long int id;
    std::string login_from;
    std::string login_to;
    std::string message;
    bool delivered;
};

// Структура для хранения информации об отложенном сообщении
struct DelayedMessage {
    unsigned long int id;
    std::string login_from;
    std::string login_to;
    std::string message;
    bool delivered;
};

// Структура для хранения информации о пользователе
struct User {
    unsigned long int id;
    std::string login;
    std::string password;
};

auto main() -> int {
    // Хранилище сообщений
    std::map<unsigned long int, std::vector<Message>> all_messages;
    // Хранилище отложенных сообщений
    std::map<unsigned long int, std::vector<DelayedMessage>> delayed_messages;
    // Хранилище пользователей
    std::map<std::string, User> all_users;
    // Хранилище активных сессий
    std::map<unsigned long int, User> active_sessions;

    httplib::Server svr; // Создаем HTTP-сервер

    // Обработчик для создания пользователя
    svr.Post("/users", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "# POST /users" << std::endl;
        nlohmann::json j = nlohmann::json::parse(req.body);

        // Проверяем наличие обязательных полей
        if (!j.contains("login") || !j.contains("password")) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing required fields\"}", "application/json");
            return;
        }

        // Проверяем, существует ли пользователь с таким логином
        if (all_users.find(j["login"]) != std::end(all_users)) {
            res.status = 400;
            res.set_content("{\"error\": \"User already created\"}", "application/json");
            return;
        }

        // Создаем нового пользователя
        User usr{all_users.size() + 1, j["login"], j["password"]};
        all_users[usr.login] = usr;

        // Формируем JSON-ответ
        nlohmann::json response;
        response["message"] = "User info received";
        response["user_info"] = {
            {"login", usr.login},
            {"password", usr.password},
            {"id", usr.id}
        };
        res.set_content(response.dump(4), "application/json");
    });

    // Обработчик для получения списка всех пользователей
    svr.Get("/users", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "# GET /users" << std::endl;
        nlohmann::json j_array = json::array();

        // Итерируемся по всем пользователям
        for (const auto &[login, user] : all_users) {
            nlohmann::json j_object = {
                {"login", login},
                {"id", user.id}
            };
            j_array.push_back(j_object);
        }

        res.set_content(j_array.dump(4), "application/json");
    });

    // Обработчик для создания сессии (входа пользователя)
    svr.Post("/session", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "# POST /session" << std::endl;
        nlohmann::json j = nlohmann::json::parse(req.body);

        // Проверяем наличие обязательных полей
        if (!j.contains("login") || !j.contains("password")) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing required fields\"}", "application/json");
            return;
        }

        // Проверяем, существует ли пользователь с таким логином
        if (all_users.find(j["login"]) == std::end(all_users)) {
            res.status = 400;
            res.set_content("{\"error\": \"Unknown user\"}", "application/json");
            return;
        }

        // Проверяем пароль
        if (all_users[j["login"]].password != j["password"]) {
            res.status = 403;
            res.set_content("{\"error\": \"Wrong password\"}", "application/json");
            return;
        }

        // Создаем новую сессию
        unsigned long session_id = active_sessions.size() + 1;
        active_sessions[session_id] = all_users[j["login"]];

        // Доставляем отложенные сообщения
        User user = active_sessions[session_id];
        if (delayed_messages.find(user.id) != std::end(delayed_messages)) {
            auto &messages = delayed_messages[user.id];
            for (auto &msg : messages) {
                if (!msg.delivered) {
                    // Добавляем сообщение в обычные сообщения
                    all_messages[user.id].push_back({msg.id, msg.login_from, msg.login_to, msg.message, false});
                    msg.delivered = true;
                }
            }
        }

        // Формируем JSON-ответ
        nlohmann::json response;
        response["Session Info"] = {
            {"session id", session_id},
            {"login", active_sessions[session_id].login}
        };
        res.set_content(response.dump(4), "application/json");
    });

    // Обработчик для отправки сообщения
    svr.Post("/messages", [&](const httplib::Request &req, httplib::Response &res) {
        static unsigned long message_id = 0;

        std::cout << "# POST /messages" << std::endl;
        nlohmann::json j = nlohmann::json::parse(req.body);

        // Проверяем наличие обязательных полей
        if (!j.contains("session") || !j.contains("to") || !j.contains("message")) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing required fields\"}", "application/json");
            return;
        }

        // Проверяем, существует ли сессия
        if (active_sessions.find(j["session"]) == std::end(active_sessions)) {
            res.status = 400;
            res.set_content("{\"error\": \"Wrong session\"}", "application/json");
            return;
        }

        // Проверяем, существует ли получатель
        if (all_users.find(j["to"]) == std::end(all_users)) {
            res.status = 400;
            res.set_content("{\"error\": \"Wrong destination user\"}", "application/json");
            return;
        }

        // Получаем информацию о отправителе и получателе
        User user_from = active_sessions[j["session"]];
        User user_to = all_users[j["to"]];

        // Создаем новое сообщение
        if (all_messages.find(user_to.id) == std::end(all_messages)) {
            all_messages[user_to.id] = std::vector<Message>();
        }

        all_messages[user_to.id].push_back({++message_id, user_from.login, user_to.login, j["message"], false});

        // Формируем JSON-ответ
        nlohmann::json response;
        response["Message Info"] = {
            {"message id", message_id},
            {"from user", user_from.id},
            {"to user", user_to.id}
        };
        res.set_content(response.dump(4), "application/json");
    });

    // Обработчик для отправки отложенного сообщения
    svr.Post("/delayed_messages", [&](const httplib::Request &req, httplib::Response &res) {
        static unsigned long delayed_message_id = 0;

        std::cout << "# POST /delayed_messages" << std::endl;
        nlohmann::json j = nlohmann::json::parse(req.body);

        // Проверяем наличие обязательных полей
        if (!j.contains("session") || !j.contains("to") || !j.contains("message")) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing required fields\"}", "application/json");
            return;
        }

        // Проверяем, существует ли сессия
        if (active_sessions.find(j["session"]) == std::end(active_sessions)) {
            res.status = 400;
            res.set_content("{\"error\": \"Wrong session\"}", "application/json");
            return;
        }

        // Проверяем, существует ли получатель
        if (all_users.find(j["to"]) == std::end(all_users)) {
            res.status = 400;
            res.set_content("{\"error\": \"Wrong destination user\"}", "application/json");
            return;
        }

        // Получаем информацию о отправителе и получателе
        User user_from = active_sessions[j["session"]];
        User user_to = all_users[j["to"]];

        // Создаем новое отложенное сообщение
        delayed_messages[user_to.id].push_back({++delayed_message_id, user_from.login, user_to.login, j["message"], false});

        // Формируем JSON-ответ
        nlohmann::json response;
        response["Delayed Message Info"] = {
            {"message id", delayed_message_id},
            {"from user", user_from.id},
            {"to user", user_to.id}
        };
        res.set_content(response.dump(4), "application/json");
    });

    // Обработчик для получения сообщений
    svr.Get("/messages", [&](const httplib::Request &req, httplib::Response &res) {
        std::cout << "# GET /messages" << std::endl;

        // Проверяем наличие обязательных параметров
        if (!req.has_param("session") || !req.has_param("all")) {
            res.status = 400;
            res.set_content("{\"error\": \"Missing required fields\"}", "application/json");
            return;
        }

        // Получаем параметры запроса
        std::string all = req.get_param_value("all");
        long int session = std::stoi(req.get_param_value("session"));

        // Проверяем, существует ли сессия
        if (active_sessions.find(session) == std::end(active_sessions)) {
            res.status = 400;
            res.set_content("{\"error\": \"Wrong session\"}", "application/json");
            return;
        }

        // Получаем информацию о пользователе
        User user_to = active_sessions[session];
        nlohmann::json j_array = json::array();

        // Проверяем, есть ли сообщения для пользователя
        if (all_messages.find(user_to.id) != std::end(all_messages)) {
            bool read_all = (all == "true");
            auto &messages = all_messages[user_to.id];
            for (auto &msg : messages) {
                if (read_all || !msg.delivered) {
                    // Формируем JSON-объект для сообщения
                    nlohmann::json j_object = {
                        {"id", msg.id},
                        {"login_from", msg.login_from},
                        {"login_to", msg.login_to},
                        {"message", msg.message}
                    };
                    j_array.push_back(j_object);
                    msg.delivered = true;
                }
            }
        }

        // Возвращаем JSON-ответ
        res.set_content(j_array.dump(4), "application/json");
    });

    // Запуск сервера
    std::cout << "starting chat server on port 8000 ..." << std::endl;
    svr.listen("0.0.0.0", 8000);
    return 0;
}