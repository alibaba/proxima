use test_db;

insert into test_forward_with_numeric values(5, 125, 20005, 65605, 2000000005, 8000000005, 5.1234, 5.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_numeric values(6, 126, 20006, 65606, 2000000006, 8000000006, 6.1234, 6.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_numeric values(7, 127, 20007, 65607, 2000000007, 8000000007, 7.1234, 7.11223344, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');

delete from test_forward_with_numeric where id = 1;
update test_forward_with_numeric set f2 = f2 + 1 where id > 0;

select count(*) from test_forward_with_numeric;
