<?php

$currentVersion = $_GET['version'];
$mac = $_GET['mac'];

$recentVersion['84:F3:EB:7A:9C:5B'] = 1005;	// WeMos 1 - napelemen lévő
$recentVersion['84:F3:EB:7A:E3:AA'] = 1004;	// WeMos 2

$baseFile = "weather.ino.bin";

if ($_SERVER['HTTP_USER_AGENT'] == "ESP8266-http-Update" )
{
	if (isset($recentVersion[$mac]) && $recentVersion[$mac] > $currentVersion)
	{
		$nextVersion = $recentVersion[$mac];
		$file = $baseFile . "." . $nextVersion;
		header('Content-Description: File Transfer');
		header('Content-Type: application/octet-stream');
		header('Content-Disposition: attachment; filename="' . basename($file) . '"');
		header('Expires: 0');
		header('Cache-Control: must-revalidate');
		header('Pragma: public');
		header('Content-Length: ' . filesize($file));
		readfile($file);
	}
	else
	{
		// Unknown MAC address
		header('HTTP/1.0 404 Not Found');
	}
}
else
{
		header('HTTP/1.0 403 Forbidden');
}
?>
