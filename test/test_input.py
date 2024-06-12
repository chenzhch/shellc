import sys
print("0:"+sys.argv[0])
print("1:"+sys.argv[1])
print("2:"+sys.argv[2])
with open('/dev/tty', 'r') as device:
    print("input data:")
    data = device.readline()
    print("data is "+str(data))
