// Factorial calculation in Vypr

// Function to calculate factorial
func factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)

// Main program
print "Factorial Calculator"
print "----------------"

var num
print "Enter a number:"
input num

// Convert input to integer
num = parseInt(num)

if num < 0:
    print "Factorial is not defined for negative numbers."
else:
    var result = factorial(num)
    print "Factorial of " ^ num ^ " is " ^ result

// Calculate first 10 factorials
print "\nFirst 10 factorials:"
loop i in [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]:
    print i ^ "! = " ^ factorial(i)

// Alternative way to loop N times
print "\nUsing loop N times:"
loop 5 times:
    print "Hello!" 