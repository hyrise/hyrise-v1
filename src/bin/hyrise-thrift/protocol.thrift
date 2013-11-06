struct Query {
       1: required string query
       2: optional i64 session_context
       3: optional bool autocommit = false
       4: optional i64 limit = 0
       5: optional i64 offset = 0
}

struct Result {
       1: required string result
}

service Hyrise {

	/*
	Allows to ping the service and check if its up
	*/
	void ping()

	Result query(1: Query q)
}