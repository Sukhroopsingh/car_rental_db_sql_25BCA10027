# Car Rental Management System — C++ OOP + MySQL

This is a console-based OOP mini-project. It stores its records in a real MySQL database (not in temporary variables or a text file).

## Features

1. Add and view cars.
2. Add and view customers.
3. Rent an available car to a registered customer.
4. View rental history and active rentals.
5. Return a car and make it available again.
6. Uses prepared statements for data input and a transaction for rental/return updates.

## Software needed on Windows

- MySQL Server and (recommended) MySQL Workbench.
- MySQL Connector/C++ **Developer** and runtime components for Windows.
- Visual Studio 2022 with **Desktop development with C++**, including CMake tools.
- CMake 3.20 or later.

Official downloads:
- MySQL Community downloads: https://dev.mysql.com/downloads/
- MySQL Connector/C++: https://dev.mysql.com/downloads/connector/cpp/
- CMake: https://cmake.org/download/
- Visual Studio: https://visualstudio.microsoft.com/downloads/

Use the Windows connector package built for the MSVC toolchain. The official Windows binaries are intended for Visual Studio/MSVC, not MinGW g++.

## Setup

### 1. Create the database tables

1. Start the MySQL Server service.
2. Open MySQL Workbench and connect to your local server.
3. Open `car_rental_db.sql` and execute the whole script.
4. In the SCHEMAS panel, refresh and confirm that `car_rental_db` now contains `cars`, `customers`, and `rentals`.

### 2. Set the password in the C++ code

Open `main.cpp`. Change:

`const std::string DB_PASSWORD = "CHANGE_THIS_PASSWORD";`

to the password you chose for your local MySQL `root` account. Keep the password private and do not upload it publicly.

### 3. Build with CMake and Visual Studio's C++ compiler

Open **Developer PowerShell for Visual Studio 2022** or a terminal configured to use the Visual Studio C++ compiler. Change directory to this project folder.

Find the Connector/C++ installation folder. It should be the directory containing `mysql-concpp-config.cmake`. Then run (replace the example path with your actual folder):

```powershell
cmake -S . -B build -Dmysql-concpp_DIR="C:/path/to/mysql-connector-c++"
cmake --build build --config Release
```

If the connector was installed system-wide and CMake can already find it, you can omit the `-Dmysql-concpp_DIR=...` argument.

### 4. Run the app

For a standard Visual Studio multi-configuration build:

```powershell
.\build\Release\CarRental.exe
```

If Windows reports that a connector DLL cannot be found, add the Connector/C++ runtime library folder to your `PATH` for that terminal, or copy the connector runtime DLLs beside `CarRental.exe`. Keep the connector's required runtime dependencies available as described in the connector's installation notes.

## How the connection works

1. MySQL Server runs locally and listens on `127.0.0.1:3306`.
2. The C++ Connector/C++ library lets the program communicate with the server.
3. `CarRentalSystem` creates a connection using the host, username and password constants in `main.cpp`.
4. Each menu action sends a prepared SQL statement such as `INSERT`, `SELECT`, or `UPDATE`.
5. MySQL stores the information in the database files managed by the MySQL Server. Closing the C++ program does not delete saved records.

## Check that records were stored

After using the program to add records, run these in MySQL Workbench:

```sql
USE car_rental_db;
SELECT * FROM cars;
SELECT * FROM customers;
SELECT * FROM rentals;
```

## Troubleshooting

- **Access denied for user root**: check `DB_USER` and `DB_PASSWORD` in `main.cpp`.
- **Unknown database car_rental_db**: run `car_rental_db.sql` in Workbench.
- **Can't connect / connection refused**: start the MySQL Server service and confirm port 3306.
- **mysql/jdbc.h not found**: install the Connector/C++ Developer component and configure CMake to find its install folder.
- **Library/compiler errors with MinGW**: the official Windows Connector/C++ binaries target MSVC. Build this project with Visual Studio's MSVC toolchain, or use a SQLite-based version for a MinGW-only setup.
