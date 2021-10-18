create database if not exists test_db;
use test_db;

drop table if exists test_forward_with_bit;
create table if not exists test_forward_with_bit(id int primary key auto_increment, f1 bit(8), f2 bit(16), f3 bit(24), f4 bit(48), f5 float, column1 varchar(256));

insert into test_forward_with_bit values(1, 0, 128, 65535, 5000000000, 1.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]');
insert into test_forward_with_bit values(2, 1, 129, 65536, 5000000001, 2.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]');
insert into test_forward_with_bit values(3, 2, 130, 65537, 5000000002, 3.0, '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]');


# select id, f1+0,f2+0,f3+0,f4+0,f5,column1 from test_forward_with_bit;
select count(*) from test_forward_with_bit;
