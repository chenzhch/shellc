add()
{
echo ` expr $1 + $2 `
}
echo  "input x:"
read x </dev/tty
echo "input y:"
read y </dev/tty
echo "$x add $y result:" 
add $x $y
