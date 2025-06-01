#include "crow_all.h"
#include <iostream>
#include <stdexcept>
#include <pqxx/pqxx>
#include <optional>

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
        }
        catch (const exception &e) {
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
        }
        catch (const exception &e) {
            cerr << "Database select error: " << e.what() << endl;
            throw;
        }
    }

    bool insertUser(const string& name, const string& email) {
        try {
            pqxx::connection c(connection_string);
            pqxx::work w(c);
            
            w.exec_params("INSERT INTO users (name, email) VALUES ($1, $2)", name, email);
            w.commit();
            
            return true;
        }
        catch (const exception &e) {
            cerr << "Database insert error: " << e.what() << endl;
            return false;
        }
    }
};

int main() {
    DatabaseManager db;
    
    // Initialize database on startup
    try {
        db.initializeDatabase();
    }
    catch (const exception &e) {
        cerr << "Failed to initialize database. Exiting." << endl;
        return 1;
    }

    crow::SimpleApp app;

    // GET endpoint - retrieve all users
    CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::Get)
        ([&db]() {
            try {
                crow::json::wvalue users = db.getAllUsers();
                return users;
            }
            catch (const exception &e) {
                return error2json("Internal server error");
            }
        });

    // POST endpoint - insert new user
    CROW_ROUTE(app, "/users").methods(crow::HTTPMethod::Post) ([&db](const crow::request& req) {
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
            }
            catch (const exception &e) {
                return error2json("Internal server error");
            }
        });

    // Root endpoint
    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::Get) ([]() {
            crow::json::wvalue response;
            response["message"] = "REST API with PostgreSQL - Use /users endpoint";
            return response;
        });

    cout << "Starting server on port 8080..." << endl;

    app.port(8080).multithreaded().run();
    return 0;
}