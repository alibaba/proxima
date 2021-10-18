create database if not exists test_db;
use test_db;
drop table if exists full_table_with_empty_table;
create table if not exists full_table_with_empty_table(id int primary key auto_increment, name varchar(10), col_a float, col_b int, column1 varchar(256));
