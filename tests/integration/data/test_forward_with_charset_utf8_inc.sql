use test_db;

insert into test_forward_with_charset_utf8 values(5, '第一个字段', '定长字段', '我是谁', '第三个字段', 5.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,5]');
insert into test_forward_with_charset_utf8 values(6, '第一个字段', '定长字段', '我是谁', '第三个字段', 6.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,6]');
insert into test_forward_with_charset_utf8 values(7, '第一个字段', '定长字段', '我是谁', '第三个字段', 7.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,7]');

update test_forward_with_charset_utf8 set f5 = f5 + 1;

delete from test_forward_with_charset_utf8 where id=7;

select count(*) from test_forward_with_charset_utf8;
