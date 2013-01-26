ALTER TABLE creature_proto CHANGE COLUMN `power` `minpower` INT(10) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE creature_proto ADD COLUMN `maxpower` INT(10) UNSIGNED NOT NULL DEFAULT '0' after minpower;
UPDATE creature_proto SET maxpower = minpower;