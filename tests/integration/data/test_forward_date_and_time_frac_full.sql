create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_date_and_time_frac;
create table if not exists test_forward_with_date_and_time_frac(id int primary key auto_increment, f1 date, f2 time(1), f3 datetime(3), f4 timestamp(6) DEFAULT CURRENT_TIMESTAMP(6), f5 float, f6 year, column1 varchar(256));

insert into test_forward_with_date_and_time_frac values(1, '2021-01-12', '13:00:01.9', '2021-01-12 13:00:01.123', '2021-01-12 13:00:00.123456', 1.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_date_and_time_frac values(2, '2021-01-13', '13:00:02.9', '2021-01-13 13:00:02.123', '2021-01-13 13:00:00.123456', 2.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_date_and_time_frac values(3, '2021-01-14', '13:00:03.9', '2021-01-14 13:00:03.123', '2021-01-14 13:00:00.123456', 3.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_date_and_time_frac values(4, '2021-01-15', '13:00:04.9', '2021-01-15 13:00:04.123', '2021-01-15 13:00:00.123456', 4.1, '2021','[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_forward_with_date_and_time_frac;
