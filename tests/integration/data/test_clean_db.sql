create database if not exists test_db;
use test_db;
drop table if exists test_collection;
create table if not exists test_collection(id int primary key auto_increment, name varchar(10), col_a float, col_b int, column1 varchar(256));
