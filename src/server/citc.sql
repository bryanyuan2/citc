CREATE TABLE IF NOT EXISTS `corpus` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `crc` bigint(20) unsigned NOT NULL COMMENT 'DEPRECATED',
  `phrase` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `syllable` varchar(255) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `priority` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `crc` (`crc`),
  KEY `phone` (`syllable`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=133680 ;

CREATE TABLE IF NOT EXISTS `candidate` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `phrase` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `syllable` varchar(255) CHARACTER SET ascii COLLATE ascii_bin DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `phrase` (`phrase`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=133697 ;
