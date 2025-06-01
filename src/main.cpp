#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>

#include "crow_all.h"

using namespace std;

crow::json::wvalue error2json(const string &errstring) {
    crow::json::wvalue r;
    r["error"] = errstring;
    return r;
}

class DatabaseManager {
   private:
    string connection_string;

   public:
    DatabaseManager() : connection_string("dbname=mydb user=postgres password=password hostaddr=127.0.0.1 port=5432") {}

    void initializeDatabase() {
        try {
            pqxx::connection c(connection_string);
            pqxx::work w(c);

            // Create table if it doesn't exist
            w.exec(R"(
                CREATE TABLE IF NOT EXISTS users (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(100) NOT NULL,
                    email VARCHAR(100) UNIQUE NOT NULL,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            )");

            w.commit();
            cout << "Database initialized successfully" << endl;
        } catch (const exception &e) {
            cerr << "Database initialization error: " << e.what() << endl;
            throw;
        }
    }

    crow::json::wvalue getAllUsers() {
        try {
            pqxx::connection c(connection_string);
            pqxx::nontransaction w(c);

            pqxx::result r = w.exec("SELECT id, name, email, created_at FROM users ORDER BY id");

            crow::json::wvalue users;
            users = crow::json::wvalue::list();

            int i = 0;
            for (auto row : r) {
                crow::json::wvalue user;
                user["id"] = row[0].as<int>();
                user["name"] = row[1].as<string>();
                user["email"] = row[2].as<string>();
                user["created_at"] = row[3].as<string>();
                users[i++] = move(user);
            }

            return users;
        } catch (const exception &e) {
            cerr << "Database select error: " << e.what() << endl;
            throw;
        }
    }

    bool insertUser(const string &name, const string &email) {
        try {
            pqxx::connection c(connection_string);
            pqxx::work w(c);

            w.exec_params("INSERT INTO users (name, email) VALUES ($1, $2)", name, email);
            w.commit();

            return true;
        } catch (const exception &e) {
            cerr << "Database insert error: " << e.what() << endl;
            return false;
        }
    }

    bool deleteUser(int id) {
        try {
            pqxx::connection c(connection_string);
            pqxx::work w(c);

            auto result = w.exec_params("DELETE FROM users WHERE id = $1", id);
            w.commit();

            return result.affected_rows() > 0;
        } catch (const exception &e) {
            cerr << "Database delete error: " << e.what() << endl;
            return false;
        }
    }

    bool updateUser(int id, const string &name, const string &email) {
        try {
            pqxx::connection c(connection_string);
            pqxx::work w(c);

            auto result = w.exec_params("UPDATE users SET name = $1, email = $2 WHERE id = $3", name, email, id);
            w.commit();

            return result.affected_rows() > 0;
        } catch (const exception &e) {
            cerr << "Database update error: " << e.what() << endl;
            return false;
        }
    }

    optional<crow::json::wvalue> getUserById(int id) {
        try {
            pqxx::connection c(connection_string);
            pqxx::nontransaction w(c);

            pqxx::result r = w.exec_params("SELECT id, name, email, created_at FROM users WHERE id = $1", id);

            if (r.empty()) {
                return nullopt;
            }

            crow::json::wvalue user;
            auto row = r[0];
            user["id"] = row[0].as<int>();
            user["name"] = row[1].as<string>();
            user["email"] = row[2].as<string>();
            user["created_at"] = row[3].as<string>();

            return user;
        } catch (const exception &e) {
            cerr << "Database select error: " << e.what() << endl;
            throw;
        }
    }
};

int main() {
    DatabaseManager db;

    // Initialize database on startup
    try {
        db.initializeDatabase();
    } catch (const exception &e) {
        cerr << "Failed to initialize database. Exiting." << endl;
        return 1;
    }

    crow::SimpleApp app;
    crow::mustache::set_global_base("../templates");

    // GET endpoint - retrieve all users
    CROW_ROUTE(app, "/users").methods("GET"_method)([&db]() {
        try {
            crow::json::wvalue users = db.getAllUsers();
            return users;
        } catch (const exception &e) {
            return error2json("Internal server error");
        }
    });

    // POST endpoint - insert new user
    CROW_ROUTE(app, "/users").methods("POST"_method)([&db](const crow::request &req) {
        try {
            auto json_data = crow::json::load(req.body);

            if (!json_data) {
                return error2json("Invalid JSON");
            }

            if (!json_data.has("name") || !json_data.has("email")) {
                return error2json("Missing required fields: name, email");
            }

            string name = json_data["name"].s();
            string email = json_data["email"].s();

            if (db.insertUser(name, email)) {
                crow::json::wvalue response;
                response["message"] = "User created successfully";
                return response;
            } else {
                return error2json("Failed to create user");
            }
        } catch (const exception &e) {
            return error2json("Internal server error");
        }
    });

    // GET endpoint - retrieve user by ID
    CROW_ROUTE(app, "/users/<int>").methods("GET"_method)([&db](int id) {
        try {
            auto user_opt = db.getUserById(id);
            if (!user_opt.has_value()) {
                return error2json("User not found");
            }
            return user_opt.value();
        } catch (const exception &e) {
            return error2json("Internal server error");
        }
    });

    // PUT endpoint - update user
    CROW_ROUTE(app, "/users/<int>").methods("PUT"_method)([&db](const crow::request &req, int id) {
        try {
            auto json_data = crow::json::load(req.body);

            if (!json_data) {
                return error2json("Invalid JSON");
            }

            if (!json_data.has("name") || !json_data.has("email")) {
                return error2json("Missing required fields: name, email");
            }

            string name = json_data["name"].s();
            string email = json_data["email"].s();

            if (db.updateUser(id, name, email)) {
                crow::json::wvalue response;
                response["message"] = "User updated successfully";
                return response;
            } else {
                return error2json("Failed to update user or user not found");
            }
        } catch (const exception &e) {
            return error2json("Internal server error");
        }
    });

    // DELETE endpoint - delete user
    CROW_ROUTE(app, "/users/<int>").methods("DELETE"_method)([&db](int id) {
        try {
            if (db.deleteUser(id)) {
                crow::json::wvalue response;
                response["message"] = "User deleted successfully";
                return response;
            } else {
                return error2json("Failed to delete user or user not found");
            }
        } catch (const exception &e) {
            return error2json("Internal server error");
        }
    });

    // Root endpoint
    CROW_ROUTE(app, "/")([]() {
        auto page = crow::mustache::load_text("page.html");
        return page;
    });

    cout << "Starting server on port 8080..." << endl;

    app.port(8080).multithreaded().run();
    return 0;
}