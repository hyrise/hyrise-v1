##################
Loading from MySQL
##################


Loading CSV into MySQL
======================

(optional.)

Creating the MySQL Table::

    CREATE TABLE students (
        name VARCHAR(50) NOT NULL,
        matrikelnummer int(6) NOT NULL,
        wohnort VARCHAR(30),
        note DOUBLE PRECISION(2,1)
    )

Import Data from CSV File::

    LOAD DATA INFILE 'students.csv'
    INTO TABLE students
    fields terminated by ','
    lines terminated by '\n'
    (name, matrikelnummer, wohnort, note)


Loading Data from MySQL-DBs
===========================

First, make sure you have WITH_MYSQL enabled in your build configuration file settings.mk.default -> see :ref:`builprep`.

You might have to install additional dependencies -> see :ref:`sqldep`.

Before starting up your Hyrise Server, set the following environment variables to be able to connect to your mysql database::

    export HYRISE_MYSQL_HOST = host 
    export HYRISE_MYSQL_USER = username
    export HYRISE_MYSQL_PASS = password
    export HYRISE_MYSQL_PORT = db port (default 3306)

Your JSON Query to load from a mysql database might look something like this::
    
    {
    	"operators": {
    		"0": {
    			"type": "MySQLTableLoad",
    			"table": "students",
    			"database": "epic"
    		},
    		"1": {
    			"type": "SimpleTableScan",
    			"predicates": [
    				{"type": 2, "in": 0, "f": "matrikelnummer", "value": 703600}
    			]
    		}
    	},
    	"edges": [["0", "1"]]
    }


In ``"MySQLTableLoad"``, you need to specify the ``"database":`` you're going to extract data from, as well as the ``"table":`` where your data is located.

In this particular example, were loading a table of students from a MySQL Database (the example data is located at test/students.csv). We then run a Selection on that table, selecting all students, with student-id numbers greater than 703600. The result from the server should look similar to this::
    
    {
    	"header" : [ "name", "matrikelnummer", "wohnort", "note" ],
    	"performanceData" : 
    	[
    		...
    	],
    	"rows" : 
    	[
    		[ "Janina Geisler", 703601, "Berlin", 2.0 ],
    		[ "Florian Steinberg", 703602, "Berlin", 2.0 ],
    		[ "Ute Maelzer", 703603, "Berlin", 3.0 ],
    		[ "Sophia Loeffler", 703604, "Potsdam", 2.299999952316284 ],
    		[ "Renï¿½ Zobel", 703605, "Berlin", 2.0 ],
    		[ "Sabine Kohl", 703606, "Berlin", 2.0 ],
    		[ "Yvonne Kusch", 703607, "Berlin", 3.0 ],
    		[ "Dirk Krein", 703608, "Potsdam", 3.0 ],
    		[ "Maik Tintzmann", 703609, "Potsdam", 3.299999952316284 ],
    		[ "Dominik Mueller", 703610, "Frohnau", 1.299999952316284 ],
    		[ "Patrick Neureuther", 703611, "Potsdam", 1.299999952316284 ],
    		[ "Vanessa Rogge", 703612, "Berlin", 1.299999952316284 ],
    		[ "Uta Budig", 703613, "Berlin", 1.700000047683716 ],
    		[ "Diana Walter", 703614, "Berlin", 2.0 ],
    		[ "Ralf Weller", 703615, "Berlin", 2.700000047683716 ],
    		[ "Daniela Haering", 703616, "Potsdam", 2.0 ],
    		[ "Ulrich Kobelt", 703617, "Berlin", 2.0 ],
    		[ "Thorsten Girschner", 703618, "Berlin", 2.700000047683716 ],
    		[ "Felix Trueb", 703619, "Berlin", 1.299999952316284 ],
    		[ "Jan Kade", 703620, "Potsdam", 2.700000047683716 ],
    		[ "Franziska Kensy", 703621, "Potsdam", 2.0 ],
    		[ "Sophia Ladeck", 703622, "Frohnau", 1.700000047683716 ],
    		[ "Florian Wesack", 703623, "Potsdam", 2.299999952316284 ],
    		[ "Daniela Weimer", 703624, "Berlin", 3.299999952316284 ],
    		[ "Lukas Ruppersberger", 703625, "Berlin", 1.299999952316284 ]
    	]
    }
