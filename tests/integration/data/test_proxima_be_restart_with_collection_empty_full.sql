use test_db;

insert into test_proxima_be_restart_with_collection_empty values(1, 'name1', 11.111, 100, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');

select count(*) from test_proxima_be_restart_with_collection_empty;
