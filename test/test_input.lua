print("0:", arg[0])
print("1:", arg[1])
print("2:", arg[2])
local device = io.open('/dev/tty', 'r')
io.write("input data:")
local data = device:read()
print("data is ", data)
