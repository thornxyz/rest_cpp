#include "crow_all.h"
#include <iostream>
#include <stdexcept>
#include <pqxx/pqxx>
#include <json/json.h>

using namespace std;

class DatabaseManager {
private:
    string connection_string;

public:
    DatabaseManager() : connection_string("dbname=myapp user=postgres password=password hostaddr=127.0.0.1 port=5432") {}

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

    Json::Value getAllUsers() {
        try {
            pqxx::connection c(connection_string);
            pqxx::nontransaction w(c);
            
            pqxx::result r = w.exec("SELECT id, name, email, created_at FROM users ORDER BY id");
            
            Json::Value users(Json::arrayValue);
            for (auto row : r) {
                Json::Value user;
                user["id"] = row[0].as<int>();
                user["name"] = row[1].as<string>();
                user["email"] = row[2].as<string>();
                user["created_at"] = row[3].as<string>();
                users.append(user);
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
    CROW_ROUTE(app, "/users").methods("GET"_method)([&db](){
        try {
            Json::Value users = db.getAllUsers();
            Json::StreamWriterBuilder builder;
            string result = Json::writeString(builder, users);
            
            crow::response res(200, result);
            res.add_header("Content-Type", "application/json");
            return res;
        }
        catch (const exception &e) {
            crow::response res(500);
            res.write("Internal server error");
            return res;
        }
    });

    // POST endpoint - insert new user
    CROW_ROUTE(app, "/users").methods("POST"_method)([&db](const crow::request& req){
        try {
            Json::Reader reader;
            Json::Value root;
            
            if (!reader.parse(req.body, root)) {
                crow::response res(400);
                res.write("Invalid JSON");
                return res;
            }
            
            if (!root.isMember("name") || !root.isMember("email")) {
                crow::response res(400);
                res.write("Missing required fields: name, email");
                return res;
            }
            
            string name = root["name"].asString();
            string email = root["email"].asString();
            
            if (db.insertUser(name, email)) {
                crow::response res(201);
                res.write("User created successfully");
                return res;
            } else {
                crow::response res(500);
                res.write("Failed to create user");
                return res;
            }
        }
        catch (const exception &e) {
            crow::response res(500);
            res.write("Internal server error");
            return res;
        }
    });

    // Root endpoint
    CROW_ROUTE(app, "/")([](){
        return "REST API with PostgreSQL - Use /users endpoint";
    });

    cout << "Starting server on port 18080..." << endl;
    app.port(18080).multithreaded().run();
    
    return 0;
}