create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_charset_gbk;
create table if not exists test_forward_with_charset_gbk(id int primary key auto_increment, f1 char(16), f2 char(255), f3 varchar(3), f4 varchar(1024), f5 float, column1 varchar(256)) default charset gbk;

insert into test_forward_with_charset_gbk values(1, '第一个字段', '定长字段', '我是谁', '第三个字段', 1.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_charset_gbk values(2, '第一个字段', '定长字段', '我是谁', '第三个字段', 2.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_charset_gbk values(3, '第一个字段', '定长字段', '我是谁', '第三个字段', 3.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_charset_gbk values(4, '第一个字段', '定长字段', '我是谁', '第三个字段', 4.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');

select count(*) from test_forward_with_charset_gbk;
