USE world;

DROP TABLE IF EXISTS `t_users`;
CREATE TABLE `t_users` (
	`id` INT(8) UNSIGNED NOT NULL AUTO_INCREMENT,
	`name` varchar(32) DEFAULT NULL,
	`status` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`tm_date` SMALLINT(4) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_user_info`;
CREATE TABLE `t_user_info` (
  `user_id` bigint(16) NOT NULL AUTO_INCREMENT,
  `ezplay_id` varchar(64) DEFAULT NULL,
  `user_pwd` varchar(100) DEFAULT '111111',
  `user_status` varchar(10) DEFAULT '0',
  `last_login_ip` varchar(15) DEFAULT '0.0.0.0',
  `user_email` varchar(64) DEFAULT 'default@x-spaces.com',
  `last_login_ts` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `login_times` bigint(16) DEFAULT '0',
  `user_from` int(2) DEFAULT '0',
  PRIMARY KEY (`user_id`),
  KEY `ezplay_id` (`ezplay_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1200 DEFAULT CHARSET=utf8;
