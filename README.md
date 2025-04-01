# Introducing: Vypr

A simple programming language compiler that translates Vypr code (.vy files) to Python. Vypr offers a clean syntax inspired by a combination of Python and Javascript, but with some extra unique features for readability and simplicity.

## Project Structure

```
vypr/
├── include/                  # Header files
│   ├── token.h               # Token definition
│   ├── lexer.h               # Lexical analyzer
│   ├── ast.h                 # Abstract Syntax Tree nodes
│   ├── parser.h              # Syntax analyzer
│   ├── semantic_analyzer.h   # Semantic analyzer
│   ├── ir_generator.h        # Intermediate representation generator
│   ├── code_generator.h      # Python code generator
│   └── compiler.h            # Main compiler driver
├── src/                      # Source files
│   ├── token.cpp             # Token implementation
│   ├── lexer.cpp             # Lexical analyzer implementation
│   ├── parser.cpp            # Syntax analyzer implementation
│   ├── semantic_analyzer.cpp # Semantic analyzer implementation
│   ├── ir_generator.cpp      # IR generator implementation
│   ├── code_generator.cpp    # Python code generator implementation
│   ├── compiler.cpp          # Compiler driver implementation
│   └── main.cpp              # Main executable entry point
├── examples/                 # Example Vypr programs
│   ├── sample.vy             # Demonstration of all basic Vypr features
│   └── function_test.vy      # Demonstration of functions in Vypr
├── build/                    # Build directory (created during build)
├── CMakeLists.txt            # CMake build configuration
└── README.md                 # This file
```

## Building the Compiler

### Prerequisites

- C++17 compatible compiler (GCC, Clang, MSVC)
- CMake 3.12 or higher

### Build Instructions

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/vypr.git
   cd vypr
   ```

2. Create a build directory and configure:
   ```
   mkdir build
   cd build
   cmake ..
   ```

3. Build the project:
   ```
   cmake --build .
   ```

4. The executable will be available in the `build/bin` directory.

## Usage

### Running Vypr Programs on Windows

(Assuming you're in the same directory as the compiled `vypr.exe` file...)

1. Compile a Vypr program:
   ```
   vypr.exe path/to/program.vy
   ```

2. This will create a Python file with the same name (`program.py`) and a batch file to run it. It will also automatically run the generated Python file upon compilation.

3. You can also create an executable with a specific name:
   ```
   vypr.exe -o my_program path/to/program.vy
   ```

4. If you want to rerun the program without compiling Vypr code again, do this:
   ```
   my_program.bat
   ```

### Command-line Options

- `-v, --verbose`: Show compilation progress (organized from lexical analysis stage to code generation / IR stage) 
- `-o filename`: Specify output .exe file name
- `-h, --help`: Show help message

## Vypr Language Documentation

### Basic Syntax

Vypr uses a clean, Python + JS hybrid-like syntax with some unique features:

#### Variable Declaration and Assignment

Variables are declared using the `var` keyword:

```
var name = "World"
var age = 25
var pi = 3.14
var isActive = true
```

#### Data Types

Vypr supports the following primitive data types:
- Strings: `"Hello"` or `'Hello'`
- Integers: `42`
- Floating Point: `3.14`
- Booleans: `true` or `false`
- Arrays: `[1, 2, 3]`

#### Operators

Arithmetic operators:
```
var a = 5 + 3    // Addition
var b = 5 - 3    // Subtraction
var c = 5 * 3    // Multiplication
var d = 5 / 3    // Division
var e = 5 % 3    // Modulo
```

String concatenation:
```
var greeting = "Hello, " ^ name ^ "!"
```

Comparison operators:
```
var isEqual = a == b
var isNotEqual = a != b
var isGreater = a > b
var isLess = a < b
var isGreaterOrEqual = a >= b
var isLessOrEqual = a <= b
```

Logical operators (short-circuiting):
```
var bothTrue = (a > 0) && (b < 10) // Logical AND
var eitherTrue = (a > 0) || (b < 10) // Logical OR
```

Operator Precedence (Highest to Lowest):

1. `()` (Parentheses)
2. `*`, `/`, `%` (Multiplication, Division, Modulo)
3. `+`, `-` (Addition, Subtraction)
4. `^` (String Concatenation)
5. `<`, `<=`, `>`, `>=` (Comparison)
6. `==`, `!=` (Equality)
7. `&&` (Logical AND)
8. `||` (Logical OR)
9. `!` (Logical NOT)
10. `=` (Assignment)

#### Control Flow

If-else statements:

```
if age >= 18:
    print "You are an adult"
else if age <= 10:
    print "You are a toddler"
else:
    print "You are a teenage minor"
```

#### Loops

For loops (iterating over items):

```
var numbers = [1, 2, 3, 4, 5]
loop num in numbers:
    print "Number: " ^ num
```

Times loops (repeat a specific number of times):

```
loop 5 times:
    print "Hello!"
```

While loops:

```
var count = 0
while count < 5:
    print "Count: " ^ count
    count = count + 1
```

#### Functions

Function declaration:

```
func greet(name):
    print "Hello, " ^ name ^ "!"

func add(a, b):
    return a + b
```

Function calls:

```
greet("World")
var result = add(3, 5)
print "Result: " ^ result
```

#### Input and Output

Printing to console:

```
print "Hello, World!"
```

Getting user input:

```
var name
input name
print "Hello, " ^ name
```

#### Arrays and Operations

Array creation:

```
var fruits = ["apple", "banana", "orange"]
```

Accessing array elements (0-based indexing):

```
var firstFruit = fruits[0]  // "apple"
```

### Complete Examples

Check the `examples/` directory for complete code examples.

## Language Implementation Details

The Vypr compiler consists of several stages:

1. **Lexical Analysis**: The `lexer.cpp` module tokenizes the source code into tokens.
2. **Syntax Analysis**: The `parser.cpp` module builds an Abstract Syntax Tree (AST) from the tokens.
3. **Semantic Analysis**: The `semantic_analyzer.cpp` module checks for semantic errors.
4. **IR Generation**: The `ir_generator.cpp` module converts the AST to an Intermediate Representation (IR).
5. **Code Generation**: The `code_generator.cpp` module generates Python code from the IR.

## License

This project is licensed under the MIT License. See the LICENSE file for details. 

## Developers

This PL was made with love by team AGILE of TN32:
- Dane Quintano
- Jansen Moral
- JC Paglinawan
- Dharmveer Sandhu