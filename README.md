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

## API Endpoints

- **GET /** - API information
- **GET /users** - Retrieve all users
- **POST /users** - Create a new user
