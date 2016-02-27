<?


class db {

    var $link;						/* mssql의 연결 identifier */
    var $result;					/* mssql의 쿼리 결과 identifier */
    var $get_table_name_id;


	var $hostName;
	var $userName;
	var $password;


	/* link ID */
	function db($dbName, $hostname, $username, $passwd) {

		$this->hostName = $hostname;
		$this->userName = $username;
		$this->password = $passwd;

		/* DB 연결 */
		if(!($this->link = mssql_connect($this->hostName,$this->userName,$this->password))) {

			$this->printError(sprintf("error connecting to host %s, by user %s", $this->hostName,$this->userName));
            $this->printError(sprintf("error: %d %s",mssql_errno($this->link),mssql_error($this->link)));
			exit();
		}

        /* DB 선택 */
		if(!mssql_select_db($dbName,$this->link)) {
			$this->printError(sprintf("error in selecting %s database", $dbName));
			$this->printError(sprintf("error : %d , %s", mssql_errno($this->link), mssql_error($this->link)));
			exit();
		}
	}

	function close() {
		if (!$this->link)
		mssql_close($this->link);
	}


	/* result ID */

	function query($query) {
	     if(!($this->result = mssql_query($query,$this->link))) {
		       $this->printError(sprintf("error in excuting %s stmt", $query));
			   $this->printError(sprintf("error : %d , %s", mssql_errno($this->link), mssql_error($this->link)));
			   exit();
         }
		 return $this->result;
	}

	function list_tables($dbName) {

  	     if(!($this->result = mssql_list_tables($dbName))) {
		       $this->printError(sprintf("Error in listing %s table", $dbName));
		       $this->printError(sprintf("error : %d , %s", mssql_errno($this->link), mssql_error($this->link)));
		       exit();
	     }
	}




    /* Elements */

    function getTableName($offset) {

		 if($this->getNumRow() > $offset)
 		       return mssql_tablename($this->result,$offset);
		 else
			   return 0;
	}

	function existTable($dbName,$tableName) {

		 $this->list_tables($dbName);
		 if($this->result) {
              for ($i=0; $table_name = $this->getTableName($i); $i++) {
				  if($table_name == $tableName)
					  return 1;
				  else
					  return 0;
              }
		 }
		 return 0;
	}

    function data_seek ($no)
    {
      return mssql_data_seek ($this->result, $no);
    }

    function getNumRow() {
         return mssql_num_rows($this->result);
	}

    function fetch() {
         return mssql_fetch_object($this->result);
    }

	function printError($errorMesg) {
		 printf("<BR> %s <BR> \n",$errorMesg);
	}
}




// mysql
class mysql_db {

    var $link;						/* mysql의 연결 identifier */
    var $result;					/* mysql의 쿼리 결과 identifier */
    var $get_table_name_id;


	var $hostName;
	var $userName;
	var $password;


	/* link ID */
	function mysql_db($dbName, $hostname, $username, $passwd) {

		$this->hostName = $hostname;
		$this->userName = $username;
		$this->password = $passwd;

		/* DB 연결 */
		if(!($this->link = mysql_connect($this->hostName,$this->userName,$this->password))) {

			$this->printError(sprintf("error connecting to host %s, by user %s", $this->hostName,$this->userName));
            $this->printError(sprintf("error: %d %s",mysql_errno($this->link),mysql_error($this->link)));
			exit();
		}

        /* DB 선택 */
		if(!mysql_select_db($dbName,$this->link)) {
			$this->printError(sprintf("error in selecting %s database", $dbName));
			$this->printError(sprintf("error : %d , %s", mysql_errno($this->link), mysql_error($this->link)));
			exit();
		}
	}

	function close() {
		if (!$this->link)
		mysql_close($this->link);
	}


	/* result ID */

	function query($query) {
	     if(!($this->result = mysql_query($query,$this->link))) {
		       $this->printError(sprintf("error in excuting %s stmt", $query));
			   $this->printError(sprintf("error : %d , %s", mysql_errno($this->link), mysql_error($this->link)));
			   exit();
         }
		 return $this->result;
	}

	function list_tables($dbName) {

  	     if(!($this->result = mysql_list_tables($dbName))) {
		       $this->printError(sprintf("Error in listing %s table", $dbName));
		       $this->printError(sprintf("error : %d , %s", mysql_errno($this->link), mysql_error($this->link)));
		       exit();
	     }
	}




    /* Elements */

    function getTableName($offset) {

		 if($this->getNumRow() > $offset)
 		       return mysql_tablename($this->result,$offset);
		 else
			   return 0;
	}

	function existTable($dbName,$tableName) {

		 $this->list_tables($dbName);
		 if($this->result) {
              for ($i=0; $table_name = $this->getTableName($i); $i++) {
				  if($table_name == $tableName)
					  return 1;
				  else
					  return 0;
              }
		 }
		 return 0;
	}

    function data_seek ($no)
    {
      return mysql_data_seek ($this->result, $no);
    }

    function getNumRow() {
         return mysql_num_rows($this->result);
	}

    function fetch() {
         return mysql_fetch_object($this->result);
    }

	function printError($errorMesg) {
		 printf("<BR> %s <BR> \n",$errorMesg);
	}
}

?>