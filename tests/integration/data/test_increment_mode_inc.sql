use test_db;

insert into test_increment_mode values(3, 'name3', 13.111, 300, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_increment_mode values(4, 'name4', 14.111, 400, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');
insert into test_increment_mode values(5, 'name5', 15.111, 500, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_increment_mode values(6, 'name6', 16.111, 600, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_increment_mode values(7, 'name7', 17.111, 700, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');

select count(*) from test_increment_mode;
