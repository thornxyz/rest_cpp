# C++ REST API with PostgreSQL

A simple REST API built with C++ using the Crow framework and PostgreSQL database. This application provides endpoints for user management with CRUD operations.

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
- **POST /users** - Create a new user (accepts JSON: `{"name": "string", "email": "string"}`)

All API endpoints are available on **http://localhost:8080**
