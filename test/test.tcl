puts "0:[info script]"
set num1 [lindex $argv 0]
set num2 [lindex $argv 1]
set result [expr $num1 + $num2]
puts "$num1 + $num2 = $result"
