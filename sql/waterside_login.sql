SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

DROP DATABASE IF EXISTS `waterside_login`;
CREATE DATABASE `waterside_login` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;
USE `waterside_login`;

-- ----------------------------
-- Table structure for userinfo_t
-- ----------------------------
DROP TABLE IF EXISTS `userinfo_t`;
CREATE TABLE `userinfo_t`  (
  `id` int(0) NOT NULL AUTO_INCREMENT COMMENT '用户ID',
  `create_time` datetime(0) NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `account` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '账号',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `index_account`(`account`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci COMMENT = '用户信息表' ROW_FORMAT = Dynamic;

-- ----------------------------
-- Procedure structure for account_login
-- ----------------------------
DROP PROCEDURE IF EXISTS `account_login`;
delimiter ;;
CREATE PROCEDURE `account_login`(IN `acc` varchar(255))
BEGIN
	DECLARE `userid` INT DEFAULT 0;
	SELECT `id` INTO `userid` FROM userinfo_t WHERE `account`=`acc`;
	IF `userid` = 0 THEN
		INSERT INTO userinfo_t (`account`) VALUES(`acc`);
		SET `userid` = LAST_INSERT_ID();
	END IF;
	SELECT `userid`;
END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
