use test_db;

insert into test_proxima_be_restart values(5, 'name5', 15.111, 500, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_proxima_be_restart values(6, 'name6', 16.111, 600, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');

select count(*) from test_proxima_be_restart;
