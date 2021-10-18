use test_db;

insert into test_proxima_be_restart_with_collection_empty values(2, 'name2', 12.111, 200, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');

select count(*) from test_proxima_be_restart_with_collection_empty;
