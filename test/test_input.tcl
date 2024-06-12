puts "0:[info script]"
puts "1:[lindex $argv 0]"
puts "2:[lindex $argv 1]"
set device [open "/dev/tty" r]
puts "input data:"
gets $device data
puts "data is $data"
