########################################################################
#  face 512 database
create database vts_face_fp32_512d;
use vts_face_fp32_512d;

DROP TABLE IF EXISTS vts_face_fp32_512d;
create table vts_face_fp32_512d (
  id int(11) primary key not null unique auto_increment,
  vector varchar(10240) not null,
  p_key bigint not null,
  text_field varchar(20),
  float_field float not null
) CHARSET=utf8 AUTO_INCREMENT=1;



load data infile '/var/lib/mysql-files/segment.000' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.001' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.002' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.003' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.004' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.005' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.006' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.007' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.008' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.009' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.010' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.011' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.012' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.013' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.014' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.015' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.016' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.017' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/segment.018' ignore into table vts_face_fp32_512d character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);


DROP TABLE IF EXISTS vts_face_fp32_512d_2w;
create table vts_face_fp32_512d_2w (
                                    id int(11) primary key not null unique auto_increment,
                                    vector varchar(10240) not null,
                                    p_key bigint not null,
                                    text_field varchar(20),
                                    float_field float not null
) CHARSET=utf8 AUTO_INCREMENT=1;

load data infile '/var/lib/mysql-files/segment.2w.000' ignore into table vts_face_fp32_512d_2w character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);


DROP TABLE IF EXISTS vts_face_fp32_512d_1m;
create table vts_face_fp32_512d_1m (
                                       id int(11) primary key not null unique auto_increment,
                                       vector varchar(10240) not null,
                                       p_key bigint not null,
                                       text_field varchar(20),
                                       float_field float not null
) CHARSET=utf8 AUTO_INCREMENT=1;

load data infile '/var/lib/mysql-files/segment.000' ignore into table vts_face_fp32_512d_1m character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);


DROP TABLE IF EXISTS vts_face_fp32_512d_q;
create table vts_face_fp32_512d_q (
                                       id int(11) primary key not null unique auto_increment,
                                       vector varchar(10240) not null,
                                       p_key bigint not null
) CHARSET=utf8 AUTO_INCREMENT=1;

load data infile '/var/lib/mysql-files/query.000' ignore into table vts_face_fp32_512d_q character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`);


########################################################################
#  50M 512D database
create database chinese_address_512d_l2_norm;
use chinese_address_512d_l2_norm;

DROP TABLE IF EXISTS chinese_address_512d_l2_norm;
create table chinese_address_512d_l2_norm (
                                    id int(11) primary key not null unique auto_increment,
                                    vector varchar(10240) not null,
                                    p_key bigint not null,
                                    text_field varchar(20),
                                    float_field float not null
) CHARSET=utf8 AUTO_INCREMENT=1;

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.000' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.001' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.002' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.003' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.004' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.005' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.006' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.007' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.008' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.009' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.010' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.011' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.012' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.013' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.014' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.015' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.016' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.017' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.018' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.019' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.020' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.021' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.022' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.023' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.024' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.025' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.026' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.027' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.028' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.029' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.030' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.031' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.032' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.033' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.034' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.035' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.036' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.037' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.038' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.039' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);


load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.040' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.041' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.042' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.043' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.044' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.045' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.046' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.047' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.048' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.049' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.050' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.051' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.052' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.053' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.054' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);
load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/segment.055' ignore into table chinese_address_512d_l2_norm character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`,`text_field`, `float_field`);


DROP TABLE IF EXISTS chinese_address_512d_l2_norm_q;
create table chinese_address_512d_l2_norm_q (
                                      id int(11) primary key not null unique auto_increment,
                                      vector varchar(10240) not null,
                                      p_key bigint not null
) CHARSET=utf8 AUTO_INCREMENT=1;

load data infile '/var/lib/mysql-files/chinese_address_512d_l2_norm/query.000' ignore into table chinese_address_512d_l2_norm_q character set utf8 fields
terminated by '|' enclosed by '"' lines terminated by '\n' (`vector`,`p_key`);

########################################################################

INSERT INTO vts_face_fp32_512d_q(vector, p_key) VALUES ('-0.018950,-0.027601,0.029433,-0.049073,-0.022660,-0.005965,0.056194,0.005138,0.055736,-0.017441,-0.015001,-0.036576,0.004568,-0.058825,-0.071009,0.006936,0.041904,0.020544,0.029058,0.016218,-0.037882,0.008780,-0.010335,-0.003099,0.079163,0.064205,-0.003475,0.001644,0.015865,-0.005466,-0.027281,0.008134,-0.071116,-0.021186,0.025063,-0.072102,-0.058192,-0.053226,0.058055,-0.043831,0.019077,-0.044566,-0.008920,0.067477,0.005366,0.053768,0.064434,0.006666,0.002357,0.021475,-0.002751,-0.012222,-0.051546,0.013340,-0.019127,0.023062,-0.015804,-0.070216,0.016976,-0.056982,-0.010817,-0.056350,0.014693,-0.017572,0.100992,-0.001988,0.026432,-0.081943,0.018010,0.000795,-0.023242,0.020200,-0.040964,-0.035147,-0.008796,-0.011831,-0.051079,0.031011,-0.057911,0.039357,0.045563,-0.020473,-0.014889,0.021077,-0.087319,-0.050683,-0.000506,-0.015053,0.063425,0.023479,0.013168,-0.049454,-0.023739,-0.039333,0.035981,-0.039312,-0.039243,0.001949,0.016238,-0.133142,-0.039268,0.066276,0.018204,-0.033430,-0.047440,0.022615,0.000749,0.083299,-0.047930,0.008380,-0.082127,0.014617,0.001402,0.082150,-0.001830,0.018488,0.000286,0.053385,-0.089946,0.021997,0.021003,0.027360,0.036879,-0.059475,-0.018871,0.092225,0.029489,-0.026508,-0.015768,-0.012865,0.088295,0.016123,-0.029778,-0.102396,0.005653,0.033438,0.076318,0.047537,-0.019370,0.003880,-0.038357,-0.069244,0.013206,0.052731,-0.044422,-0.045984,0.064287,0.063458,0.012366,0.046554,-0.014541,-0.046981,0.040710,0.007809,0.019138,-0.031280,0.005744,-0.032424,0.013631,0.032215,-0.045975,0.002801,0.115970,-0.013436,-0.035482,-0.006199,0.078901,-0.064859,0.005653,-0.051042,0.008569,0.044996,0.010914,0.070450,-0.046443,0.003210,-0.026466,0.028877,-0.051071,-0.020633,0.051578,-0.120661,-0.010065,-0.023436,0.001714,-0.032687,-0.016210,0.005997,-0.082633,0.097420,-0.031321,0.029058,0.060846,0.062699,-0.024812,0.038409,-0.007182,0.021995,0.030892,-0.007325,-0.077771,-0.006569,-0.070532,-0.021422,0.049408,-0.109623,0.005212,-0.004861,0.012130,0.043313,0.009163,0.050616,-0.040313,-0.002631,-0.001109,0.001306,-0.022754,-0.021270,-0.056147,0.051881,0.031744,-0.020617,0.016948,0.025522,0.016708,-0.035161,0.053157,-0.002037,-0.003167,0.111554,-0.002936,-0.028547,-0.095569,0.004574,0.050479,-0.032712,-0.010738,0.060238,-0.038399,0.042237,0.021586,-0.083791,-0.020363,-0.017915,-0.001482,-0.038230,0.015712,-0.006684,0.031344,-0.068242,0.020323,0.002729,0.025776,-0.061503,0.010611,0.017804,0.041669,0.012392,-0.060341,-0.024900,-0.063002,-0.001920,-0.032915,-0.000960,-0.021514,-0.014564,0.061115,-0.076246,-0.062856,-0.071194,-0.021202,-0.016131,-0.059900,-0.034610,-0.040206,-0.042895,-0.002483,0.044605,0.001372,-0.101529,-0.001828,-0.103048,-0.031297,-0.016878,-0.023834,0.050420,0.024020,-0.011137,-0.140238,-0.051979,0.085020,0.038933,0.127042,0.041524,-0.030117,0.056891,-0.053334,-0.030602,0.011756,0.007324,-0.030329,-0.062977,-0.042806,-0.007362,0.021153,-0.025529,-0.021425,0.036511,0.010076,-0.065501,0.007580,0.092202,-0.019660,-0.006779,-0.014505,-0.011312,-0.048175,0.008486,0.047491,0.037514,-0.027510,-0.039198,-0.100382,-0.045332,0.016991,-0.006936,-0.017366,-0.007444,0.014395,-0.003089,-0.020629,0.027420,0.024758,0.070364,-0.011739,-0.032171,-0.019401,0.007725,-0.013883,-0.045235,0.054859,0.061361,-0.064525,0.064236,0.056047,-0.064166,-0.031746,-0.001632,-0.031432,-0.063762,-0.031150,-0.018962,-0.014152,0.030340,0.027059,0.049580,-0.025282,-0.031288,0.013674,0.060906,0.045000,0.019587,-0.023220,-0.081773,-0.027007,0.018261,-0.000590,-0.040861,-0.105557,0.015284,0.028116,-0.032138,-0.002735,0.066251,0.046955,0.054924,0.023915,0.109000,-0.056548,0.015768,-0.075634,-0.020457,-0.028878,-0.046798,-0.057080,0.057820,0.037019,0.038066,-0.009608,-0.039675,0.005541,0.028341,-0.005972,0.053169,0.018158,0.067248,0.012660,0.055927,0.031064,0.021389,-0.080137,-0.026971,-0.028203,-0.037743,-0.015084,0.049724,-0.077273,0.035753,-0.000662,0.001476,-0.046561,-0.060630,0.031582,0.057681,0.037149,0.012713,-0.017181,-0.043256,0.003465,0.022548,0.052186,-0.040553,0.064455,0.021539,0.018883,-0.069429,-0.049200,0.070659,0.077909,-0.020314,0.082661,-0.024631,0.018384,0.056514,-0.051840,0.033918,-0.048232,-0.007297,-0.031624,-0.117258,-0.053612,-0.009953,-0.031192,0.043974,-0.008840,-0.015087,0.042430,-0.065707,0.056052,-0.028191,0.049596,-0.002935,-0.000412,0.019444,0.055688,0.109548,0.038258,0.014887,-0.033741,0.042629,-0.022666,-0.032125,-0.009331,0.059250,0.009829,-0.028745,-0.025815,-0.010050,0.011845,0.103870,0.020677,0.052920,-0.001595,-0.023385,-0.020915,-0.023902,0.039025,-0.056825,-0.063770,-0.001545,-0.023381,-0.013910,0.016631,0.021308,0.067967,-0.011235,0.056421,-0.001273,-0.027336,0.009527,0.042496,0.022707,0.083721,-0.012626,-0.085037,-0.001673,-0.028829,-0.013406,0.024951,0.048866,-0.011498,-0.059011,-0.011989,-0.016756,0.029857,0.103094,0.007111,-0.041760,0.002769,0.029121,-0.013594,0.004342', 4);
INSERT INTO vts_face_fp32_512d_q(vector, p_key) VALUES ('0.018990,-0.026564,-0.074380,-0.080027,-0.003530,0.020116,-0.026819,-0.055056,-0.050954,-0.016818,-0.055611,-0.018200,-0.050677,-0.049803,-0.047470,0.088019,-0.090301,0.005176,0.089186,0.005646,-0.033093,-0.019510,-0.000758,-0.022662,-0.072344,-0.022143,-0.065633,0.007557,-0.051194,0.081988,0.003223,-0.025066,0.042486,-0.040949,0.018456,0.074975,-0.039089,-0.055742,-0.001078,0.054048,0.039184,0.046202,-0.071246,-0.037686,0.010271,-0.114977,-0.009068,0.002448,-0.053331,-0.012330,-0.015430,-0.014888,0.041111,-0.013474,0.034354,0.003789,0.083562,0.041563,0.008410,0.037660,0.028192,-0.005239,0.051109,-0.000177,0.052671,0.017819,-0.036006,0.034817,-0.022381,0.031019,0.061063,-0.018029,0.031324,0.060098,0.126787,0.023183,-0.016489,-0.007297,0.070049,0.025467,0.029072,0.001847,0.001005,0.117192,-0.035301,-0.037476,0.038708,-0.007165,0.016045,0.017104,0.079098,0.023868,0.001864,0.009841,-0.019286,-0.030859,-0.003673,0.029864,-0.002053,0.086085,0.019369,-0.011575,-0.043770,0.058725,0.073420,0.081043,0.028588,0.010871,-0.035738,-0.039326,0.037432,-0.037647,0.001318,0.043523,-0.018898,-0.042326,0.010487,-0.047589,-0.056921,-0.011206,-0.018926,0.027976,-0.040548,0.031734,0.012047,-0.011499,-0.021265,-0.030023,0.039084,0.000235,0.002119,-0.035474,-0.059109,-0.001139,0.000803,-0.026916,-0.007739,-0.063737,-0.052308,-0.074166,0.013420,0.004411,-0.017026,-0.084907,-0.000459,0.018069,0.015364,-0.046284,-0.051097,-0.065892,-0.016638,-0.035136,0.021947,-0.089489,0.033304,0.026275,0.072900,0.086350,-0.094363,0.022757,-0.000078,0.014630,-0.015252,0.048451,-0.022585,-0.057376,0.032287,-0.044275,-0.012600,-0.003670,-0.011928,-0.094233,-0.057325,0.015122,0.066657,0.081942,0.036027,0.090712,0.076276,-0.017960,-0.010482,0.087801,0.136267,-0.074674,0.029554,0.003440,-0.043471,-0.006148,-0.038998,0.028725,0.066128,-0.029892,-0.005003,-0.063408,-0.006527,-0.067585,-0.012122,-0.023724,0.007523,0.028675,-0.010489,0.019769,0.024188,-0.086028,-0.009872,0.006107,0.025676,0.044236,-0.038995,-0.009083,0.003050,-0.079702,-0.015497,0.006418,-0.051633,0.015834,0.065541,0.006776,0.053002,0.014452,-0.004406,0.006974,0.014991,-0.005093,0.037989,-0.083318,-0.099467,0.019911,-0.001837,-0.078542,0.013706,0.077943,0.077858,-0.037037,0.000138,-0.010079,-0.128671,0.002789,0.035846,-0.002067,-0.082037,0.002213,-0.011649,-0.026202,0.081602,-0.030421,-0.021853,-0.014350,-0.030303,-0.003272,-0.026875,0.005095,-0.021909,0.029722,-0.021974,0.028804,-0.032812,0.047766,0.009548,0.055889,0.020871,0.013290,0.009591,-0.056730,0.107562,0.004044,-0.046458,0.019570,-0.005909,0.036564,-0.079987,-0.004859,-0.006975,0.050858,0.002987,0.004677,-0.046685,-0.044758,-0.024142,-0.020945,0.007080,-0.015139,0.005727,-0.040768,0.132578,0.000116,-0.014818,-0.054616,-0.036315,0.061882,-0.037466,-0.024772,0.010992,-0.033946,0.068233,0.033256,0.001423,-0.017947,-0.074121,0.038813,0.017305,0.013695,-0.024176,-0.039063,0.030579,-0.066201,0.058209,0.022739,0.074405,0.044019,-0.038122,-0.106768,0.010272,0.014302,-0.021927,0.025372,0.002067,-0.064063,-0.061733,0.036851,0.065449,-0.007891,0.013536,-0.013212,0.026221,0.049972,-0.009896,-0.024969,0.003525,0.002189,-0.039483,0.041425,0.074186,-0.010678,-0.051394,-0.006110,0.001774,-0.042279,0.037422,0.088283,0.034655,-0.001786,0.059899,0.000806,0.009341,0.068523,0.034918,0.058531,0.000901,0.003348,0.026887,-0.055659,0.040627,-0.112764,0.010084,0.080034,0.015784,-0.003562,0.009442,0.009441,0.041112,-0.014453,-0.039941,0.079657,-0.055720,-0.018789,0.077986,-0.022412,-0.018566,0.002274,0.034367,0.000179,0.024719,-0.008818,0.001224,-0.027264,0.031056,-0.007380,0.001956,-0.061412,0.071740,0.039339,0.004653,-0.034407,0.056294,0.022648,-0.032928,-0.086359,0.014597,0.075572,-0.031202,-0.053848,-0.009450,-0.047667,-0.043598,-0.025867,-0.088403,-0.011794,0.063497,-0.009792,0.032514,-0.000666,0.019377,-0.040243,0.010717,-0.060903,-0.025458,-0.032373,-0.008307,0.041381,-0.013571,0.011215,0.076445,-0.020289,-0.060025,-0.037909,0.062530,-0.095224,0.017458,0.072853,-0.061164,-0.035332,0.012541,-0.021209,0.011143,-0.025961,0.029610,0.043860,0.013443,-0.040672,-0.025607,-0.004672,-0.030812,0.011047,0.016260,0.048821,-0.033020,-0.003214,0.013867,0.075351,-0.060843,-0.005076,0.002915,0.003891,0.065470,-0.006947,0.007156,-0.035394,0.032894,-0.056455,-0.009901,0.028474,0.034317,-0.047070,-0.004093,-0.011997,0.086919,0.044756,0.031250,0.050920,0.016834,0.062481,0.009912,-0.086627,-0.010909,0.043168,-0.003410,0.033574,0.020280,-0.033200,-0.070535,0.002198,0.006363,-0.038977,0.019581,0.049575,0.004098,-0.029766,-0.003892,-0.023865,0.034137,-0.018515,-0.119172,-0.050676,0.031222,0.085486,-0.042790,0.013223,-0.013531,-0.068723,0.022705,0.088682,-0.043891,-0.011074,0.025330,0.064888,-0.058756,0.024609,-0.023462,0.044005,-0.043933,0.033105,-0.019530,0.047792,0.037710,-0.082358,-0.027842,0.049849,0.002086,0.034141,0.025395,0.035188', 2);