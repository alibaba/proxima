use test_db;

insert into test_forward_with_bit values(4, 3, 131, 65538, 5000000003, 4.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');
insert into test_forward_with_bit values(5, 4, 132, 65539, 5000000004, 5.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_bit values(6, 5, 133, 65540, 5000000005, 6.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_bit values(7, 6, 134, 65541, 5000000006, 7.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');

update test_forward_with_bit set f5 = f5 + 1;

delete from test_forward_with_bit where id=7;

# select id, f1+0,f2+0,f3+0,f4+0,f5,column1 from test_forward_with_bit;
select count(*) from test_forward_with_bit;
