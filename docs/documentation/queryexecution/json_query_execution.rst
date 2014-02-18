####################
JSON Query Execution
####################

1. start the server:


.. code-block:: sh
  :linenos:

	./build/hyrise_server -l ./build/log.properties
	
2. tell the server, where to look for Table data:

.. code-block:: sh
  :linenos:

	export HYRISE_DB_PATH='pwd'/pathToTable


3. post query to server:

.. code-block:: sh
  :linenos:

	curl -X POST --data-urlencode "query@path/to/json/test.json" http://localhost:5000/jsonQuery

4. get back query result from server