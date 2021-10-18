use test_db;

insert into test_forward_with_set_and_enum values(5, '5,8', '555', '5', '5555', 5.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_set_and_enum values(6, '6,8', '666', '6', '6666', 6.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_set_and_enum values(7, '7,8', '777', '7', '7777', 7.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');


update test_forward_with_set_and_enum set f5 = f5 + 1;

delete from test_forward_with_set_and_enum where id=7;

select count(*) from test_forward_with_set_and_enum;
