// Function Test Program

// 1. Basic function with no parameters and no return
func greet():
    print "Hello from greet()!"

// 2. Function with parameters but no return
func print_sum(a, b):
    print "Sum of " ^ a ^ " and " ^ b ^ " is: " ^ (a + b)

// 3. Function with parameters and return value
func add(a, b):
    return a + b

// 4. Function with multiple return statements
func get_sign(num):
    if num > 0:
        return "positive"
    else if num < 0:
        return "negative"
    else:
        return "zero"

// 5. Function with nested function calls
func square(x):
    return x * x

func sum_squares(a, b):
    return square(a) + square(b)

// Test the functions
print "Testing functions..."
greet()

var x = 5
var y = 3
print_sum(x, y)

var result = add(x, y)
print "add(" ^ x ^ ", " ^ y ^ ") returned: " ^ result

var num = -7
var sign = get_sign(num)
print "The number " ^ num ^ " is " ^ sign

var a = 3
var b = 4
var sum = sum_squares(a, b)
print "Sum of squares of " ^ a ^ " and " ^ b ^ " is: " ^ sum

print "Function tests complete!" 