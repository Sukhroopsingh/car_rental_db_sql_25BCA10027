CREATE DATABASE IF NOT EXISTS car_rental_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE car_rental_db;

CREATE TABLE IF NOT EXISTS cars (
    id INT AUTO_INCREMENT PRIMARY KEY,
    brand VARCHAR(80) NOT NULL,
    model VARCHAR(80) NOT NULL,
    plate_number VARCHAR(24) NOT NULL UNIQUE,
    daily_rate DECIMAL(10, 2) NOT NULL,
    is_available TINYINT(1) NOT NULL DEFAULT 1,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_car_daily_rate CHECK (daily_rate > 0)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS customers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    full_name VARCHAR(120) NOT NULL,
    phone VARCHAR(25) NOT NULL,
    license_number VARCHAR(60) NOT NULL UNIQUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS rentals (
    id INT AUTO_INCREMENT PRIMARY KEY,
    car_id INT NOT NULL,
    customer_id INT NOT NULL,
    start_date DATE NOT NULL,
    rental_days INT NOT NULL,
    total_cost DECIMAL(10, 2) NOT NULL,
    status ENUM('ACTIVE', 'RETURNED') NOT NULL DEFAULT 'ACTIVE',
    return_date DATE NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_rental_days CHECK (rental_days > 0),
    CONSTRAINT chk_rental_total CHECK (total_cost >= 0),
    CONSTRAINT fk_rental_car FOREIGN KEY (car_id)
        REFERENCES cars(id) ON UPDATE CASCADE ON DELETE RESTRICT,
    CONSTRAINT fk_rental_customer FOREIGN KEY (customer_id)
        REFERENCES customers(id) ON UPDATE CASCADE ON DELETE RESTRICT,
    INDEX idx_rentals_status (status),
    INDEX idx_rentals_car (car_id),
    INDEX idx_rentals_customer (customer_id)
) ENGINE=InnoDB;

-- Optional: inspect the saved data in MySQL Workbench with:
-- SELECT * FROM cars;
-- SELECT * FROM customers;
-- SELECT * FROM rentals;
