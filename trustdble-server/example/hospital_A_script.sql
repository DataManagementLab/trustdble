DROP SHARED DATABASE Hospital_Digital_System;
CREATE SHARED DATABASE Hospital_Digital_System type=STUB encrypted=true shards=1;

CREATE SHARED TABLE hospitals (
  id INT PRIMARY KEY,
  name VARCHAR(255) NOT NULL,
  address VARCHAR(255) NOT NULL,
  phone_number VARCHAR(20) NOT NULL,
  email VARCHAR(255) NOT NULL,
  number_of_beds INT
);

CREATE SHARED TABLE patients (
  id INT PRIMARY KEY,
  first_name VARCHAR(255) NOT NULL,
  last_name VARCHAR(255) NOT NULL,
  date_of_birth DATE NOT NULL,
  address VARCHAR(255) NOT NULL,
  phone_number VARCHAR(20) NOT NULL,
  email VARCHAR(255) NOT NULL,
  health_insurance_provider VARCHAR(255)
);

CREATE SHARED TABLE cases (
  id INT PRIMARY KEY,
  patient_id INT NOT NULL,
  hospital_id INT NOT NULL,
  case_type VARCHAR(255) NOT NULL,
  admission_date DATE NOT NULL,
  discharge_date DATE NOT NULL,
  diagnosis VARCHAR(255) NOT NULL,
  treatment VARCHAR(255) NOT NULL,
  cost INT,
  FOREIGN KEY (patient_id) REFERENCES patient(id),
  FOREIGN KEY (hospital_id) REFERENCES hospital(id)
);

INSERT INTO hospitals (id, name, address, phone_number, email)
VALUES (1, 'Main Campus', '123 Main St, Hessen GERMANY', '555-555-1234', 'main@hospital.com'),
       (2, 'North Campus', '456 North St, Hamburg GERMANY', '555-555-5678', 'north@hospital.com');

INSERT INTO patients (id, first_name, last_name, date_of_birth, address, phone_number, email, health_insurance_provider)
VALUES (1, 'John', 'Doe', '1970-01-01', '123 Elm St, Hessen GERMANY', '555-555-1111', 'john.doe@example.com', 'public-TK'),
       (2, 'Jane', 'Smith', '1980-02-02', '456 Maple St, Hessen GERMANY', '555-555-2222', 'jane.smith@example.com', 'private-DKV'),
       (3, 'Alice', 'Carry', '1982-05-04', '345 Micheale St, Hamburg GERMANY', '555-555-3333', 'alice.carry@example.com', 'private-DKV'),
       (4, 'Sara', 'Muller', '1988-08-07', '241 Dierburger St, Hessen GERMANY', '555-555-444', 'sara.muller@example.com', 'private-TK'),
       (5, 'Alex', 'Jackson', '1990-12-09', '234 Alexander St, Hamburg GERMANY', '555-555-9999', 'alex.jackson@example.com', 'public-TK');

INSERT INTO cases (id, patient_id, hospital_id, case_type, admission_date, discharge_date, diagnosis, treatment, cost)
VALUES (1, 1, 1, 'Emergency', '2022-01-01', '2022-01-03', 'Appendicitis', 'Surgery', 2000),
       (2, 1, 1, 'Routine', '2022-02-01', '2022-02-05', 'Broken Arm', 'Cast', 200),
       (3, 2, 2, 'Emergency', '2022-03-01', '2022-03-03', 'Heart Attack', 'Stent', 4000),
       (4, 2, 2, 'Routine', '2022-03-01', '2022-04-03', 'COVID19', 'Stent', 100),
       (5, 3, 1, 'Routine', '2022-03-01', '2022-04-03', 'COVID19', 'Stent', 100),
       (6, 4, 1, 'Routine', '2022-03-01', '2022-04-03', 'COVID19', 'Stent', 100),
       (7, 1, 1, 'Routine', '2022-03-01', '2022-04-03', 'COVID19', 'Stent', 100),
       (8, 5, 1, 'Routine', '2022-03-05', '2022-04-04', 'COVID19', 'Stent', 150),
       (9, 5, 2, 'Emergency', '2022-05-05', '2022-06-04', 'Heart Attack', 'Cast', 1500);