// Vypr Showcase Program

// 1. Variable Declaration and Assignment (different types)
var greeting = "Hello, Vypr Learner!"
var score = 100
var pi_val = 3.14159
var is_learning = true
var items = ["pen", "paper", "code"] // Array

// 2. Output
print greeting
print "Initial score: " ^ score
print "Is learning? " ^ is_learning

// 3. Input and Type Conversion
var user_name
print "Please enter your name: "
input user_name

var fav_num_str
print "Please enter your favorite number: "
input fav_num_str
var fav_num = int(fav_num_str) // Convert input string to integer

print "Welcome, " ^ user_name ^ "! Your favorite number is " ^ fav_num

// 4. Operators
var doubled_num = fav_num * 2
var half_num = fav_num / 2 // Note: Integer division might occur depending on Python version if not float
print fav_num ^ " doubled is " ^ doubled_num
print fav_num ^ " halved is " ^ half_num

var added_score = score + fav_num
print "Score plus favorite number: " ^ added_score

// 5. Comparisons and Conditionals (if/else if/else)
print "Checking your favorite number..."
if fav_num > 10:
    print "That's a big favorite number!"
else if fav_num == 7:
    print "Lucky number 7!"
else if fav_num < 0:
    print "A negative number, interesting!"
else:
    print "A nice, standard number."

var is_positive_and_even = (fav_num > 0) && (fav_num % 2 == 0) // Using modulo (%) implicitly - assuming it works like Python
print "Is favorite number positive and even? " ^ is_positive_and_even

// 6. Loops
// 6a. Loop In (For-each style)
print "Items in your inventory:"
loop item in items:
    print "- " ^ item

// 6b. Loop Times (Fixed repetitions)
print "Repeating 'Vypr!' 3 times:"
loop 3 times:
    print "Vypr!"

// 6c. While Loop
print "Countdown:"
var countdown = 3
while countdown > 0:
    print countdown
    countdown = countdown - 1
print "Blast off!"

// 7. Functions
func greet_person(name, number):
    print "Function says: Hello " ^ name ^ ", number " ^ number ^ "!"
    return number * 10 // Return a value

var result = greet_person(user_name, fav_num)
print "Function returned: " ^ result

// 8. Array Access
print "The second item in the list is: " ^ items[1] // Access 'paper'

// 9. Array Manipulation
items[1] = 'dane'
print "The second item in the list is changed to: " ^ items[1] // Access 'dane'

print "Showcase complete!"
