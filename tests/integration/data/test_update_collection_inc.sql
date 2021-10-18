use test_db;

insert into test_update_collection values(3, 'name3', 13.111, 300, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_update_collection values(4, 'name4', 14.111, 400, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_update_collection;
