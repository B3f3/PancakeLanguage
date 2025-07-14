# Pancake

**Pancake** is a minimalist interpreted programming language built with C++. It's designed for ease of understanding and experimentation, featuring a straightforward syntax, simple statements, and an expression parser with operator precedence.

---

## Features

- Expression Parsing with Operator Precedence
- Variable Declaration (`let`)
- Output (`out`) and Input (`in`) Statements
- Conditional Statements (`if`, `elif`, `else`)
- Integer, Boolean, String, and Double Literals

---

## To Compile
```
g++ -std=c++17 -o simplelang main.cpp parser.cpp tokenizer.cpp -o pancake
```
## To Run
to run console
```
./pancake
```
to run from cutom file
```
./pancake [filename].pnc
```

## Example Code
```
let int x = 5;
let string name = "John";
out -> x;
in <- y;
if x > 3 {
    out "x is greater than 3";
}
```
