use test_db;

insert into test_forward_with_date_and_time values(5, '2021-01-16', '13:00:05', '2021-01-16 13:00:05', '2021-01-16 13:00:00', 5.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_date_and_time values(6, '2021-01-17', '13:00:06', '2021-01-17 13:00:06', '2021-01-17 13:00:00', 6.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_date_and_time values(7, '2021-01-18', '13:00:07', '2021-01-18 13:00:07', '2021-01-18 13:00:00', 7.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');
insert into test_forward_with_date_and_time values(8, '2021-01-19', '13:00:08', '2021-01-19 13:00:08', '2021-01-19 13:00:00', 8.1, '2021', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,8]');

delete from test_forward_with_date_and_time where id = 1;

update test_forward_with_date_and_time set f5 = 1.0 where id > 0;

select count(*) from test_forward_with_date_and_time;
