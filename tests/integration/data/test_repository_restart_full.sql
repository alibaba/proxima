create database if not exists test_db;
use test_db;
drop table if exists test_repository_restart;
create table if not exists test_repository_restart(id int primary key auto_increment, name varchar(10), col_a float, col_b int, column1 varchar(256));

insert into test_repository_restart values(1, 'name1', 11.111, 100, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_repository_restart values(2, 'name2', 12.111, 200, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');

select count(*) from test_repository_restart;
