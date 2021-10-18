use test_db;

insert into test_one_field_both_index_fields values(5, '5555555555555555', 5.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_one_field_both_index_fields values(6, '6666666666666666', 6.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_one_field_both_index_fields values(7, '7777777777777777', 7.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');

update test_one_field_both_index_fields set f2 = f2 + 1;

delete from test_one_field_both_index_fields where id=7;

select count(*) from test_one_field_both_index_fields;
