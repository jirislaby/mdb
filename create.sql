CREATE USER IF NOT EXISTS 'xslaby'@'%';
CREATE USER IF NOT EXISTS 'xslaby'@'localhost';
CREATE DATABASE IF NOT EXISTS structs_66;
GRANT ALL PRIVILEGES ON structs_66.* TO 'xslaby'@'%';
GRANT ALL PRIVILEGES ON structs_66.* TO 'xslaby'@'localhost';
FLUSH PRIVILEGES;
USE structs_66;
DROP TABLE IF EXISTS sources;
CREATE TABLE sources(id INTEGER AUTO_INCREMENT PRIMARY KEY, src VARCHAR(1024) NOT NULL UNIQUE);