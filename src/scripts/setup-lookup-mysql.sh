#!/bin/bash
#
#	setup-lookup-mysql.sh
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
DBTABLE_LOOKUP="lookup"

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
#	create the lookup table
#
echo "Creating database table $DBTABLE_LOOKUP"
mysql --host=localhost --user=root $DBNAME <<EOF
CREATE TABLE \`$DBTABLE_LOOKUP\` (
	\`number\`			VARCHAR(20) NOT NULL DEFAULT '',
	\`name\`			VARCHAR(100) NOT NULL DEFAULT '',
	PRIMARY KEY (number)
) TYPE=MyISAM;
EOF
