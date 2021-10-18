create database if not exists test_db;
use test_db;
drop table if exists test_forward_with_numeric;
create table if not exists test_forward_with_numeric(id int primary key auto_increment, f1 tinyint, f2 smallint, f3 mediumint, f4 int, f5 BIGINT, f6 float, f7 double, column1 varchar(256));

insert into test_forward_with_numeric values(1, 121, 20001, 65601, 2000000001, 8000000001, 1.1234, 1.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_numeric values(2, 122, 20002, 65602, 2000000002, 8000000002, 2.1234, 2.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_numeric values(3, 123, 20003, 65603, 2000000003, 8000000003, 3.1234, 3.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_numeric values(4, 124, 20004, 65604, 2000000004, 8000000004, 4.1234, 4.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_forward_with_numeric;
