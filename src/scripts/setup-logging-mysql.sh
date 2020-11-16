#!/bin/bash
#
#	setup-logging-mysql.sh
#
#	Copyright (c) 2009 by Christian Lorenz
#
#	====================================================================
#
#	This file is part of listen2fritz.
#	
#	listen2fritz is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	listen2fritz is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with listen2fritz.  If not, see <http://www.gnu.org/licenses/>.
#	
DBNAME="Listen2Fritz"
DBUSER="fritz"
DBPASS="fritz"
DBTABLE_LOGGING="logging"

#
#	create the database itself
#
echo "Creating database $DBNAME"
mysql --host=localhost --user=root <<EOF
CREATE DATABASE $DBNAME;
EOF

echo "Granting access to $DBNAME"
mysql --host=localhost --user=root <<EOF
USE mysql;
FLUSH PRIVILEGES;
GRANT ALL ON $DBNAME.* TO "$DBUSER"@localhost IDENTIFIED BY "$DBUSER" WITH GRANT OPTION;
EOF

#
#	create the logging table
#
echo "Creating database table $DBTABLE_LOGGING"
mysql --host=localhost --user=root $DBNAME <<EOF
CREATE TABLE \`$DBTABLE_LOGGING\` (
	\`idx\`				INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	\`time\`			DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
	\`type\`			INT(3) UNSIGNED NOT NULL DEFAULT '0',
	\`incoming\`		INT(1) UNSIGNED NOT NULL DEFAULT '0',
	\`pretype\`			INT(3) UNSIGNED NOT NULL DEFAULT '0',
	\`line\`			INT(3) UNSIGNED NOT NULL DEFAULT '0',
	\`duration\`		INT(5) UNSIGNED NOT NULL DEFAULT '0',
	\`caller_number\`	VARCHAR(20) NOT NULL DEFAULT '',
	\`called_number\`	VARCHAR(20) NOT NULL DEFAULT '',
	PRIMARY KEY (\`idx\`),
	KEY (\`time\`)
) TYPE=MyISAM;
EOF
