<?php
echo "0:$argv[0]\n";
echo "1:$argv[1]\n"; 
echo "2:$argv[2]\n"; 
$device = fopen('/dev/tty', 'r');
echo "input data:";
$data = fgets($device);
echo "data is $data";
?>
