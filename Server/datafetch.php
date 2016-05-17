<?php

header('Content-Type: application/json');

		$servername = "YOURMYSQLSERVER";
		$username = "YOURUSERNAME";
		$password = "YOURPASSWORD";
		$dbname = "YOURDBNAME";
		
$conn = mysqli_connect($servername, $username, $password, $dbname);

// Check connection
if (mysqli_connect_errno($conn))
{
    echo "Failed to connect to DataBase: " . mysqli_connect_error();
}else
{
    $data_points = array();
    
    $result = mysqli_query($conn, "SELECT * FROM weather WHERE timestamp >= ((SELECT MAX(timestamp) from weather) - 60*60*24*7*5) ORDER BY timestamp ASC");
    
    while($row = mysqli_fetch_array($result))
    {        
        $point = array("x" => $row['timestamp']*1000-7200000 , "y1" => $row['temperature']-2, "y2" => $row['humidity'], "y3" => $row['pressure']/100.0, "y4" => $row['voltage'],);
        
        array_push($data_points, $point);        
    }
    
    echo json_encode($data_points, JSON_NUMERIC_CHECK);
}
mysqli_close($conn);

?>
