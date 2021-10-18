create database if not exists test_db;
use test_db;

drop table if exists test_one_field_both_index_and_forward;
create table if not exists test_one_field_both_index_and_forward(id int primary key auto_increment, f1 char(16), f2 float, column1 varchar(256), column2 varchar(256));

insert into test_one_field_both_index_and_forward values(1, '1111111111111111', 1.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_one_field_both_index_and_forward values(2, '2222222222222222', 2.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_one_field_both_index_and_forward values(3, '3333333333333333', 3.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_one_field_both_index_and_forward values(4, '4444444444444444', 4.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_one_field_both_index_and_forward;
