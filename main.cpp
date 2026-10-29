#include <mysql/jdbc.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

// Update these values to match your local MySQL installation.
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "root";
const std::string DB_PASSWORD = "CHANGE_THIS_PASSWORD";
const std::string DB_NAME = "car_rental_db";

class CarRentalSystem {
private:
    std::unique_ptr<sql::Connection> connection;

    static std::string readText(const std::string& prompt) {
        while (true) {
            std::cout << prompt;
            std::string value;
            std::getline(std::cin, value);
            if (!std::cin) {
                throw std::runtime_error("Input stream closed.");
            }
            if (value.find_first_not_of(" \t\r\n") != std::string::npos) {
                return value;
            }
            std::cout << "This field cannot be empty. Please try again.\n";
        }
    }

    static int readInt(const std::string& prompt, int minimum, int maximum) {
        while (true) {
            const std::string input = readText(prompt);
            try {
                std::size_t used = 0;
                const long value = std::stol(input, &used);
                if (used == input.size() && value >= minimum && value <= maximum) {
                    return static_cast<int>(value);
                }
            } catch (const std::exception&) {
                // Show the same validation message below.
            }
            std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
        }
    }

    static double readPositiveDouble(const std::string& prompt) {
        while (true) {
            const std::string input = readText(prompt);
            try {
                std::size_t used = 0;
                const double value = std::stod(input, &used);
                if (used == input.size() && std::isfinite(value) && value > 0.0) {
                    return value;
                }
            } catch (const std::exception&) {
                // Show the same validation message below.
            }
            std::cout << "Enter a valid number greater than zero.\n";
        }
    }

    void showCars(bool availableOnly) {
        const std::string query = availableOnly
            ? "SELECT id, brand, model, plate_number, daily_rate, is_available "
              "FROM cars WHERE is_available = 1 ORDER BY id"
            : "SELECT id, brand, model, plate_number, daily_rate, is_available "
              "FROM cars ORDER BY id";

        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(query));

        std::cout << "\n" << (availableOnly ? "AVAILABLE CARS" : "ALL CARS") << "\n";
        std::cout << std::left << std::setw(6) << "ID"
                  << std::setw(18) << "Brand"
                  << std::setw(18) << "Model"
                  << std::setw(18) << "Plate"
                  << std::setw(14) << "Rate/day"
                  << "Status\n";
        std::cout << std::string(88, '-') << "\n";

        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(6) << result->getInt("id")
                      << std::setw(18) << result->getString("brand")
                      << std::setw(18) << result->getString("model")
                      << std::setw(18) << result->getString("plate_number")
                      << std::fixed << std::setprecision(2)
                      << std::setw(14) << result->getDouble("daily_rate")
                      << (result->getInt("is_available") == 1 ? "Available" : "Rented")
                      << "\n";
        }
        if (!found) {
            std::cout << (availableOnly ? "No cars are currently available.\n" : "No cars have been added yet.\n");
        }
        std::cout << "\n";
    }

    void showRentals(bool activeOnly) {
        const std::string query = activeOnly
            ? "SELECT r.id AS rental_id, c.full_name AS customer_name, ca.brand, ca.model, "
              "ca.plate_number, r.start_date, r.rental_days, r.total_cost, r.status "
              "FROM rentals r JOIN customers c ON r.customer_id = c.id "
              "JOIN cars ca ON r.car_id = ca.id WHERE r.status = 'ACTIVE' ORDER BY r.id DESC"
            : "SELECT r.id AS rental_id, c.full_name AS customer_name, ca.brand, ca.model, "
              "ca.plate_number, r.start_date, r.rental_days, r.total_cost, r.status "
              "FROM rentals r JOIN customers c ON r.customer_id = c.id "
              "JOIN cars ca ON r.car_id = ca.id ORDER BY r.id DESC";

        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(query));

        std::cout << "\n" << (activeOnly ? "ACTIVE RENTALS" : "ALL RENTALS") << "\n";
        std::cout << std::left << std::setw(8) << "Rental ID"
                  << std::setw(22) << "Customer"
                  << std::setw(18) << "Car"
                  << std::setw(15) << "Plate"
                  << std::setw(13) << "Start date"
                  << std::setw(8) << "Days"
                  << std::setw(12) << "Total (INR)"
                  << "Status\n";
        std::cout << std::string(114, '-') << "\n";

        bool found = false;
        while (result->next()) {
            found = true;
            const std::string carName = result->getString("brand") + " " + result->getString("model");
            std::cout << std::left << std::setw(8) << result->getInt("rental_id")
                      << std::setw(22) << result->getString("customer_name")
                      << std::setw(18) << carName
                      << std::setw(15) << result->getString("plate_number")
                      << std::setw(13) << result->getString("start_date")
                      << std::setw(8) << result->getInt("rental_days")
                      << std::fixed << std::setprecision(2)
                      << std::setw(12) << result->getDouble("total_cost")
                      << result->getString("status") << "\n";
        }
        if (!found) {
            std::cout << (activeOnly ? "There are no active rentals.\n" : "No rentals have been created yet.\n");
        }
        std::cout << "\n";
    }

public:
    CarRentalSystem() {
        sql::Driver* driver = sql::mysql::get_driver_instance();
        connection.reset(driver->connect(DB_HOST, DB_USER, DB_PASSWORD));
        connection->setSchema(DB_NAME);
        std::cout << "Successfully connected to MySQL database: " << DB_NAME << "\n";
    }

    void addCar() {
        std::cout << "\n--- Add a Car ---\n";
        const std::string brand = readText("Brand: ");
        const std::string model = readText("Model: ");
        const std::string plate = readText("Registration/plate number: ");
        const double rate = readPositiveDouble("Rental price per day (INR): ");

        std::unique_ptr<sql::PreparedStatement> statement(
            connection->prepareStatement(
                "INSERT INTO cars (brand, model, plate_number, daily_rate, is_available) "
                "VALUES (?, ?, ?, ?, 1)"));
        statement->setString(1, brand);
        statement->setString(2, model);
        statement->setString(3, plate);
        statement->setDouble(4, rate);
        statement->executeUpdate();
        std::cout << "Car added and saved to MySQL.\n";
    }

    void viewCars() {
        showCars(false);
    }

    void addCustomer() {
        std::cout << "\n--- Add a Customer ---\n";
        const std::string name = readText("Full name: ");
        const std::string phone = readText("Phone number: ");
        const std::string license = readText("Driving licence number: ");

        std::unique_ptr<sql::PreparedStatement> statement(
            connection->prepareStatement(
                "INSERT INTO customers (full_name, phone, license_number) VALUES (?, ?, ?)"));
        statement->setString(1, name);
        statement->setString(2, phone);
        statement->setString(3, license);
        statement->executeUpdate();
        std::cout << "Customer added and saved to MySQL.\n";
    }

    void viewCustomers() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT id, full_name, phone, license_number FROM customers ORDER BY id"));

        std::cout << "\nCUSTOMERS\n";
        std::cout << std::left << std::setw(7) << "ID"
                  << std::setw(28) << "Full name"
                  << std::setw(20) << "Phone"
                  << "Licence number\n";
        std::cout << std::string(75, '-') << "\n";

        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(7) << result->getInt("id")
                      << std::setw(28) << result->getString("full_name")
                      << std::setw(20) << result->getString("phone")
                      << result->getString("license_number") << "\n";
        }
        if (!found) {
            std::cout << "No customers have been added yet.\n";
        }
        std::cout << "\n";
    }

    void rentCar() {
        std::cout << "\n--- Rent a Car ---\n";
        showCars(true);
        const int carId = readInt("Enter the ID of the car to rent: ", 1, std::numeric_limits<int>::max());
        const int customerId = readInt("Enter the customer's ID (add the customer first): ", 1, std::numeric_limits<int>::max());
        const int days = readInt("Number of rental days: ", 1, 3650);

        connection->setAutoCommit(false);
        try {
            double dailyRate = 0.0;
            {
                std::unique_ptr<sql::PreparedStatement> carStatement(
                    connection->prepareStatement(
                        "SELECT daily_rate FROM cars WHERE id = ? AND is_available = 1 FOR UPDATE"));
                carStatement->setInt(1, carId);
                std::unique_ptr<sql::ResultSet> carResult(carStatement->executeQuery());
                if (!carResult->next()) {
                    connection->rollback();
                    connection->setAutoCommit(true);
                    std::cout << "That car does not exist or is already rented.\n";
                    return;
                }
                dailyRate = carResult->getDouble("daily_rate");
            }

            {
                std::unique_ptr<sql::PreparedStatement> customerStatement(
                    connection->prepareStatement("SELECT id FROM customers WHERE id = ?"));
                customerStatement->setInt(1, customerId);
                std::unique_ptr<sql::ResultSet> customerResult(customerStatement->executeQuery());
                if (!customerResult->next()) {
                    connection->rollback();
                    connection->setAutoCommit(true);
                    std::cout << "Customer ID not found. Add the customer first.\n";
                    return;
                }
            }

            const double totalCost = dailyRate * days;
            {
                std::unique_ptr<sql::PreparedStatement> rentalStatement(
                    connection->prepareStatement(
                        "INSERT INTO rentals (car_id, customer_id, start_date, rental_days, total_cost, status) "
                        "VALUES (?, ?, CURRENT_DATE, ?, ?, 'ACTIVE')"));
                rentalStatement->setInt(1, carId);
                rentalStatement->setInt(2, customerId);
                rentalStatement->setInt(3, days);
                rentalStatement->setDouble(4, totalCost);
                rentalStatement->executeUpdate();
            }
            {
                std::unique_ptr<sql::PreparedStatement> updateCar(
                    connection->prepareStatement("UPDATE cars SET is_available = 0 WHERE id = ?"));
                updateCar->setInt(1, carId);
                updateCar->executeUpdate();
            }

            connection->commit();
            connection->setAutoCommit(true);
            std::cout << "Rental saved successfully. Total cost: INR "
                      << std::fixed << std::setprecision(2) << totalCost << "\n";
        } catch (...) {
            try {
                connection->rollback();
                connection->setAutoCommit(true);
            } catch (...) {
                // Keep the original error for the menu handler.
            }
            throw;
        }
    }

    void viewRentals() {
        showRentals(false);
    }

    void returnCar() {
        std::cout << "\n--- Return a Car ---\n";
        showRentals(true);
        const int rentalId = readInt("Enter the active Rental ID to return: ", 1, std::numeric_limits<int>::max());

        connection->setAutoCommit(false);
        try {
            int carId = 0;
            {
                std::unique_ptr<sql::PreparedStatement> rentalLookup(
                    connection->prepareStatement(
                        "SELECT car_id FROM rentals WHERE id = ? AND status = 'ACTIVE' FOR UPDATE"));
                rentalLookup->setInt(1, rentalId);
                std::unique_ptr<sql::ResultSet> rentalResult(rentalLookup->executeQuery());
                if (!rentalResult->next()) {
                    connection->rollback();
                    connection->setAutoCommit(true);
                    std::cout << "Active rental ID not found.\n";
                    return;
                }
                carId = rentalResult->getInt("car_id");
            }
            {
                std::unique_ptr<sql::PreparedStatement> updateRental(
                    connection->prepareStatement(
                        "UPDATE rentals SET status = 'RETURNED', return_date = CURRENT_DATE "
                        "WHERE id = ? AND status = 'ACTIVE'"));
                updateRental->setInt(1, rentalId);
                updateRental->executeUpdate();
            }
            {
                std::unique_ptr<sql::PreparedStatement> updateCar(
                    connection->prepareStatement("UPDATE cars SET is_available = 1 WHERE id = ?"));
                updateCar->setInt(1, carId);
                updateCar->executeUpdate();
            }
            connection->commit();
            connection->setAutoCommit(true);
            std::cout << "Car returned. Rental record updated in MySQL.\n";
        } catch (...) {
            try {
                connection->rollback();
                connection->setAutoCommit(true);
            } catch (...) {
                // Keep the original error for the menu handler.
            }
            throw;
        }
    }

    void run() {
        while (true) {
            std::cout << "\n========== CAR RENTAL MANAGEMENT SYSTEM ==========\n"
                      << "1. Add car\n"
                      << "2. View cars\n"
                      << "3. Add customer\n"
                      << "4. View customers\n"
                      << "5. Rent a car\n"
                      << "6. View rentals\n"
                      << "7. Return a car\n"
                      << "0. Exit\n";

            const int choice = readInt("Choose an option: ", 0, 7);
            if (choice == 0) {
                std::cout << "Goodbye. Your records remain stored in MySQL.\n";
                break;
            }

            try {
                switch (choice) {
                    case 1: addCar(); break;
                    case 2: viewCars(); break;
                    case 3: addCustomer(); break;
                    case 4: viewCustomers(); break;
                    case 5: rentCar(); break;
                    case 6: viewRentals(); break;
                    case 7: returnCar(); break;
                    default: break;
                }
            } catch (const sql::SQLException& e) {
                std::cout << "MySQL error: " << e.what() << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    }
};

int main() {
    try {
        CarRentalSystem system;
        system.run();
    } catch (const sql::SQLException& e) {
        std::cerr << "Could not connect to MySQL. Check that the MySQL service is running, "
                  << "the database car_rental_db exists, and DB_PASSWORD in main.cpp is correct.\n"
                  << "Details: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
