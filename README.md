# C++ REST API with PostgreSQL

A simple REST API server built with C++ using the Crow web framework and PostgreSQL database. Demonstrates CRUD operations, featuring a web interface.

## Prerequisites

- gcc
- cmake
- libpqxx
- docker
- docker-compose
- Crow (https://github.com/CrowCpp/Crow)

## Database Setup

Start the PostgreSQL database:

```bash
docker-compose up -d
```

## Build and run the server

```bash
chmod +x run.sh
./run.sh
```

The server will start on port 8080.

## Webpage

Once the server is running, you can access the web interface at:

- **http://localhost:8080** - User management webpage with forms to add users and view all users

## API Endpoints

The REST API endpoints are accessible at:

- **GET /** - Serves the user management webpage
- **GET /users** - Retrieve all users (JSON response)
- **GET /users/{id}** - Retrieve a specific user by ID (JSON response)
- **POST /users** - Create a new user (accepts JSON: `{"name": "string", "email": "string"}`)
- **PUT /users/{id}** - Update an existing user (accepts JSON: `{"name": "string", "email": "string"}`)
- **DELETE /users/{id}** - Delete a user by ID

All API endpoints are available on **http://localhost:8080**
