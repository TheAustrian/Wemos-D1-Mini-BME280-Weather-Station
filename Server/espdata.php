<?php

if ($_SERVER['HTTP_USER_AGENT'] == "ESP8266HTTPClient" ) {
		//ESP talking to this site
		$servername = "YOURMYSQLSERVER";
		$username = "YOURUSERNAME";
		$password = "YOURPASSWORD";
		$dbname = "DATABASENAME";

		if ( !empty($_POST) ) {
			$conn = mysqli_connect($servername, $username, $password, $dbname);

			$sql = "";
			foreach ($_POST['d'] as $row) {
				$sql .= "INSERT INTO weather (timestamp, temperature, pressure, humidity, voltage)
				VALUES ('" . $row['ti'] . "', '" . $row['t'] . "', '" . $row['p'] . "', '" . $row['h'] . "', '" . $row['v'] . "');";
			}
			mysqli_multi_query($conn, $sql);
			mysqli_close($conn);
			echo "0";
		} else {
			echo "1";
		}

} else {
echo "Only for my ESP!   " . "\n\n";
echo $_SERVER['HTTP_USER_AGENT'];
}
?>
