ALTER TABLE creature_spawns CHANGE COLUMN `standstate` `standstate` INT(10) UNSIGNED NOT NULL DEFAULT '0' after `emote_state`;
ALTER TABLE creature_spawns ADD COLUMN `death_state` INT(10) UNSIGNED NOT NULL DEFAULT '0' after `emote_state`;
ALTER TABLE creature_proto DROP COLUMN `death_state`;