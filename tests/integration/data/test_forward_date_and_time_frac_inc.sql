use test_db;

insert into test_forward_with_date_and_time_frac values(5, '2021-01-16', '13:00:05.9', '2021-01-16 13:00:05.123', '2021-01-16 13:00:00.123456', 5.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_date_and_time_frac values(6, '2021-01-17', '13:00:06.9', '2021-01-17 13:00:06.123', '2021-01-17 13:00:00.123456', 6.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_date_and_time_frac values(7, '2021-01-18', '13:00:07.9', '2021-01-18 13:00:07.123', '2021-01-18 13:00:00.123456', 7.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');
insert into test_forward_with_date_and_time_frac values(8, '2021-01-19', '13:00:08.9', '2021-01-19 13:00:08.123', '2021-01-19 13:00:00.123456', 8.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,8]');

delete from test_forward_with_date_and_time_frac where id = 1;

update test_forward_with_date_and_time_frac set f5 = 1.0 where id > 0;

select count(*) from test_forward_with_date_and_time_frac;
