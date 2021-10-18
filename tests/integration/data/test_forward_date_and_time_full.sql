create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_date_and_time;
create table if not exists test_forward_with_date_and_time(id int primary key auto_increment, f1 date, f2 time, f3 datetime, f4 timestamp DEFAULT CURRENT_TIMESTAMP, f5 float, f6 year, column1 varchar(256));

insert into test_forward_with_date_and_time values(1, '2021-01-12', '13:00:01', '2021-01-12 13:00:01', '2021-01-12 13:00:00', 1.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_date_and_time values(2, '2021-01-13', '13:00:02', '2021-01-13 13:00:02', '2021-01-13 13:00:00', 2.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_date_and_time values(3, '2021-01-14', '13:00:03', '2021-01-14 13:00:03', '2021-01-14 13:00:00', 3.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_date_and_time values(4, '2021-01-15', '13:00:04', '2021-01-15 13:00:04', '2021-01-15 13:00:00', 4.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_forward_with_date_and_time;
