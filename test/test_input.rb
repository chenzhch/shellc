puts "0:#{$0}"
puts "1:#{ARGV[0]}"
puts "2:#{ARGV[1]}"
File.open('/dev/tty', 'r') do |device|
  puts "input data:"
  data = device.gets.chomp
  puts "data is #{data} "
end
