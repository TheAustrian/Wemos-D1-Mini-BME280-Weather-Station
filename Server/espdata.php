<?php
if ($_SERVER['HTTP_USER_AGENT'] == "ESP8266HTTPClient" )
{
		foreach ($_POST["d"] as $key => $value)
		{
			$_POST["d"][$key]["time"] = date("Y-m-d H:i:s", time($value["ti"]));
		}

		#$data = print_r($_POST, true);

		$data = "";
		foreach ($_POST["d"] as $key => $value)
		{
			$data = $data . $value['id'] . ';' . $value['ti'] . ';' . $value['t'] . ';' . $value['p'] . ';' . $value['h'] . ';' . $value['v'] . ";" . $value['time'] . "\n";
		}

		file_put_contents("post.txt", $data, FILE_APPEND);

		echo "Got it!";
}
?>
