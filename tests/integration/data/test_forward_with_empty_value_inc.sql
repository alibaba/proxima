use test_db;

insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(5, '5555555555555555', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(6, '6666666666666666', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(7, '7777777777777777', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(8, '8888888888888888', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,8]');

delete from test_forward_with_empty_value where id = 1;

update test_forward_with_empty_value set f4 = id where id > 0;

update test_forward_with_empty_value set f4 = NULL where id > 0;

select count(*) from test_forward_with_empty_value;
