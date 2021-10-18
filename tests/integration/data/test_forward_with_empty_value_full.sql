create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_empty_value;
create table if not exists test_forward_with_empty_value(
       id int primary key auto_increment, 
       f1 char(16) not null, 
       f2 char(64) null default '', 
       f3 varchar(3) null default '', 
       f4 int null,
       f5 float null,
       f6 double null,
       f7 datetime(3) null,
       f8 timestamp null,
       f9 date null,
       f10 text null,
       f11 longtext null,
       f12 blob null,
       f13 bit(8) null,
       f14 binary(8) null default '',
       f15 varbinary(8) null default '',
       f16 set('1', '2', '3') null default '',
       f17 enum('1', '2', '3') null,
       f18 json null,
       f19 geometry null,
       f20 varchar(256));

insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(1, '1111111111111111', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(2, '2222222222222222', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(3, '3333333333333333', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');
insert into test_forward_with_empty_value(id, f1, f10, f11, f12, f13, f20) values(4, '4444444444444444', '', '', '', '', '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,4]');


select count(*) from test_forward_with_empty_value;
