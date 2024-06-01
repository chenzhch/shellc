<?php
echo "0:" . basename(__FILE__) . "\n";
$num1 = $argv[1] ?? 0;
$num2 = $argv[2] ?? 0;
$sum = $num1 + $num2;
echo "$num1 + $num2 = $sum\n";
?>
