# BASIC_INTERPRETER

A minimal BASIC interpreter written in C, designed to be kernel-compatible and free of complex external dependencies.

## Features
- Tokenization of BASIC statements
- Parsing and evaluation of arithmetic expressions
- Support for variables (A-Z)
- Basic operators: +, -, *, /, %, ^, !
- Parentheses support
- Simple shell interface
- Error handling and reporting

## Building

The project uses a Makefile for building:

```bash
make clean  # Clean up generated files
make all    # Build the interpreter
make test   # Run the interpreter
```

or 

```bash
make clean && make all && make test
```

## Usage

Run the interpreter:

```bash
./basic
```

Example commands:
```
PRINT 5 + 3        # Output: 8
PRINT 5!           # Output: 120 (factorial)
PRINT (5 + 3) * 2  # Output: 16
LET A = 10
PRINT A + 5        # Output: 15
```

## Contributing
Your contributions are most welcome!!!
1. Fork the repository
2. Create your new branch (`git switch -c your-new-branch-name`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin your-new-branch-name`)
5. Open a Pull Request


## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by minimal BASIC interpreters
- Built with C99 standards
- Designed for educational purposes
