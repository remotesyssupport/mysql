#### suite/funcs_1/storedproc/storedproc_10.inc
#
--source suite/funcs_1/storedproc/load_sp_tb.inc

# ==============================================================================
# (numbering from requirement document TP v1.0, Last updated: 25 Jan 2005 01:00)
#
# 3.1.10 CALL checks:
#
## 1. Ensure that a properly defined procedure can always be called, assuming the appropriate privileges exist.
#- 2. Ensure that a procedure cannot be called if the appropriate privileges do not exist.
## 3. Ensure that a function can never be called.
## 4. Ensure that a properly defined function can always be executed, assuming the appropriate privileges exist.
#- 5. Ensure that a function cannot be executed if the appropriate privileges do not exist.
## 6. Ensure that a procedure can never be executed.
## 7. Ensure that the ROW_COUNT() SQL function always returns the correct number of rows affected by the execution of a stored procedure.
## 8. Ensure that the mysql_affected_rows() C API function always returns the correct number of rows affected by the execution of a stored procedure.
#
# ==============================================================================
let $message= Section 3.1.10 - CALL checks:;
--source include/show_msg80.inc



# ------------------------------------------------------------------------------
let $message= Testcase 3.1.10.2 + 3.1.10.5:;
--source include/show_msg.inc
let $message=
2. Ensure that a procedure cannot be called if the appropriate privileges do not
   exist.
5. Ensure that a function cannot be executed if the appropriate privileges do
   not exist.;
--source include/show_msg80.inc

--disable_warnings
DROP PROCEDURE IF EXISTS sp31102;
DROP FUNCTION  IF EXISTS fn31105;
--enable_warnings

# DEFINER
create user 'user_1'@'localhost';
# INVOKER
create user 'user_2'@'localhost';

GRANT CREATE ROUTINE ON db_storedproc.* TO 'user_1'@'localhost';
GRANT SELECT         ON db_storedproc.* TO 'user_2'@'localhost';
FLUSH PRIVILEGES;

--replace_result $MASTER_MYPORT MYSQL_PORT $MASTER_MYSOCK MYSQL_SOCK
connect (user2_1, localhost, user_1, , db_storedproc);
--source suite/funcs_1/include/show_connection.inc

delimiter //;
CREATE PROCEDURE sp31102 () SQL SECURITY INVOKER
BEGIN
   SELECT * FROM db_storedproc.t1 WHERE f4=-5000 LIMIT 1;
END//
delimiter ;//

delimiter //;
CREATE FUNCTION fn31105(n INT) RETURNS INT
  BEGIN
  DECLARE res INT;
  SET res = n * n;
  RETURN res;
END//
delimiter ;//

disconnect user2_1;

--replace_result $MASTER_MYPORT MYSQL_PORT $MASTER_MYSOCK MYSQL_SOCK
connect (user2_2, localhost, user_2, , db_storedproc);
--source suite/funcs_1/include/show_connection.inc

# no privileges exist
--error 1370
CALL sp31102();
SELECT fn31105( 9 );

# now 'add' EXECUTE to INVOKER
connection default;
USE db_storedproc;
--source suite/funcs_1/include/show_connection.inc
# root can execute ...
CALL sp31102();
SELECT fn31105( 9 );
GRANT EXECUTE ON db_storedproc.* TO 'user_2'@'localhost';
FLUSH PRIVILEGES;
disconnect user2_2;

# new connection
--replace_result $MASTER_MYPORT MYSQL_PORT $MASTER_MYSOCK MYSQL_SOCK
connect (user2_3, localhost, user_2, , db_storedproc);
--source suite/funcs_1/include/show_connection.inc
CALL sp31102();
SELECT fn31105( 9 );
disconnect user2_3;

# now 'remove' SELECT from INVOKER
connection default;
USE db_storedproc;
--source suite/funcs_1/include/show_connection.inc
REVOKE EXECUTE ON db_storedproc.* FROM 'user_2'@'localhost';
FLUSH PRIVILEGES;

# root can still execute
CALL sp31102();
SELECT fn31105( 9 );

--replace_result $MASTER_MYPORT MYSQL_PORT $MASTER_MYSOCK MYSQL_SOCK
connect (user2_4, localhost, user_2, , db_storedproc);
--source suite/funcs_1/include/show_connection.inc
CALL sp31102();
SELECT fn31105( 9 );
disconnect user2_4;

# cleanup
connection default;
USE db_storedproc;

--source suite/funcs_1/include/show_connection.inc
DROP PROCEDURE sp31102;
DROP FUNCTION  fn31105;
DROP USER 'user_1'@'localhost';
DROP USER 'user_2'@'localhost';


# ------------------------------------------------------------------------------
let $message= Testcase 3.1.10.3:;
--source include/show_msg.inc
let $message=
Ensure that a function can never be called.;
--source include/show_msg80.inc

--disable_warnings
DROP FUNCTION IF EXISTS fn1;
--enable_warnings

delimiter //;
CREATE FUNCTION fn1(a int) returns int
BEGIN
    set @b = 0.9 * a;
    return @b;
END//
delimiter ;//

--error 1305
CALL fn1();

# cleanup
DROP FUNCTION fn1;


# ------------------------------------------------------------------------------
let $message= Testcase 3.1.10.6:;
--source include/show_msg.inc
let $message=
Ensure that a procedure can never be executed.;
--source include/show_msg80.inc

--disable_warnings
DROP PROCEDURE IF EXISTS sp1;
DROP FUNCTION IF EXISTS sp1;
--enable_warnings

delimiter //;
CREATE PROCEDURE sp1()
BEGIN
    SELECT * from t10;
END//
delimiter ;//

--error 1305
  SELECT sp1();

# cleanup
DROP PROCEDURE sp1;


# ------------------------------------------------------------------------------
let $message= Testcase 3.1.10.7:;
--source include/show_msg.inc
let $message=
Ensure that the ROW_COUNT() SQL function always returns the correct number of
rows affected by the execution of a stored procedure.;
--source include/show_msg80.inc

--disable_warnings
DROP PROCEDURE IF EXISTS sp_ins_1;
DROP PROCEDURE IF EXISTS sp_ins_3;
DROP PROCEDURE IF EXISTS sp_upd;
DROP PROCEDURE IF EXISTS sp_ins_upd;
--enable_warnings

CREATE TABLE temp(f1 CHAR(20),f2 CHAR(25),f3 DATE,f4 INT,f5 CHAR(25),f6 INT);
INSERT INTO temp SELECT * FROM t10;

delimiter //;
#FIXME: add to proc: SELECT row_count() 'ins';
CREATE PROCEDURE sp_ins_1()
BEGIN
  INSERT INTO temp VALUES ('abc', 'abc', '20051003', 100, 'uvw', 1000);
END//


#FIXME: add to proc: SELECT row_count() 'ins_3';
CREATE PROCEDURE sp_ins_3()
BEGIN
  INSERT INTO temp VALUES  ('abc', 'xyz', '19490523',   100, 'uvw', 1000);
  INSERT INTO temp VALUES  ('abc', 'xyz', '1989-11-09', 100, 'uvw', 1000);
  INSERT INTO temp VALUES  ('abc', 'xyz', '2005-10-24', 100, 'uvw', 1000);
END//

# FIXME: add to proc:  SELECT row_count() AS 'updated';
CREATE PROCEDURE sp_upd()
BEGIN
  UPDATE temp SET temp.f1 = 'updated' WHERE temp.f1 ='abc';
END//

# FIXME: use commented proc
# CREATE PROCEDURE sp_ins_upd()
# BEGIN
#    BEGIN
#       INSERT INTO temp VALUES  ('qwe', 'abc', '1989-11-09', 100, 'uvw', 1000);
#       INSERT INTO temp VALUES  ('qwe', 'xyz', '1998-03-26', 100, 'uvw', 1000);
#       INSERT INTO temp VALUES  ('qwe', 'abc', '2000-11-09', 100, 'uvw', 1000);
#       INSERT INTO temp VALUES  ('qwe', 'abc', '2005-11-07', 100, 'uvw', 1000);
#    END;
#    SELECT row_count() AS 'insert "qwe"';
#    SELECT COUNT( f1 ), f1 FROM temp GROUP BY f1;
#    UPDATE temp SET temp.f1 = 'updated_2' WHERE temp.f1 ='qwe' AND temp.f2 = 'abc';
#    SELECT row_count() AS 'update "qwe" AND "abc"';
# END//
CREATE PROCEDURE sp_ins_upd()
BEGIN
   BEGIN
      INSERT INTO temp VALUES  ('qwe', 'abc', '1989-11-09', 100, 'uvw', 1000);
      INSERT INTO temp VALUES  ('qwe', 'xyz', '1998-03-26', 100, 'uvw', 1000);
      INSERT INTO temp VALUES  ('qwe', 'abc', '2000-11-09', 100, 'uvw', 1000);
      INSERT INTO temp VALUES  ('qwe', 'abc', '2005-11-07', 100, 'uvw', 1000);
   END;
   SELECT COUNT( f1 ), f1 FROM temp GROUP BY f1;
   UPDATE temp SET temp.f1 = 'updated_2' WHERE temp.f1 ='qwe' AND temp.f2 = 'abc';
END//
delimiter ;//

CALL sp_ins_1();
SELECT row_count();
SELECT * FROM temp ORDER BY f4;

CALL sp_ins_3();
#FIXME: check is 1 correct here? I expect 3 for 3 inserted rows inside the procedure
SELECT row_count();
SELECT * FROM temp ORDER BY f4;

CALL sp_upd();
SELECT row_count();
SELECT * FROM temp ORDER BY f4;

#FIXME: check is 3 correct here?  I expect 7 for 4 inserted and then 3 updated rows inside the procedure
CALL sp_ins_upd();
SELECT row_count();
SELECT * FROM temp ORDER BY f4;

# cleanup
DROP PROCEDURE sp_ins_1;
DROP PROCEDURE sp_ins_3;
DROP PROCEDURE sp_upd;
DROP PROCEDURE sp_ins_upd;
DROP TABLE temp;


# ------------------------------------------------------------------------------
let $message= Testcase 3.1.10.8:;
--source include/show_msg.inc
let $message=
Ensure that the mysql_affected_rows() C API function always returns the correct 
number of rows affected by the execution of a stored procedure.;
--source include/show_msg80.inc

#FIXME: 3.1.10.8: to be added later.

# ==============================================================================
# USE the same .inc to cleanup before and after the test
--source suite/funcs_1/storedproc/cleanup_sp_tb.inc

# ==============================================================================
let $message= .                               +++ END OF SCRIPT +++;
--source include/show_msg80.inc
# ==============================================================================