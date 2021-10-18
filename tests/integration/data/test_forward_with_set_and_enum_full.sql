create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_set_and_enum;
create table if not exists test_forward_with_set_and_enum(id int primary key auto_increment, f1 set('1', '2', '3', '4', '5', '6', '7', '8'), f2 set('111','222','333','444', '555', '666', '777'), f3 enum('1','2','3','4','5','6','7','8'), f4 enum('1111', '2222', '3333', '4444', '5555', '6666', '7777'), f5 float, column1 varchar(256));

insert into test_forward_with_set_and_enum values(1, '1,8', '111', '1', '1111', 1.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_set_and_enum values(2, '2,8', '222', '2', '2222', 2.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_set_and_enum values(3, '3,8', '333', '3', '3333', 3.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_set_and_enum values(4, '4,8', '444', '4', '4444', 4.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_forward_with_set_and_enum;
