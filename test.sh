#!/bin/bash

try() {
  input="$1"
  expected="$2"

  ./0cc "$input" > tmp.s
  gcc tmp.s -o tmp
  ./tmp
  actual="$?"

  if [ "$actual" != "$expected" ]; then
    echo -e "expected: $expected\tinput: '$input'"
    echo "but got:  $actual"
    exit 1
  fi
}

try 0 0
try 42 42

try '5-5+10-10' 0
try '20+31-9' 42

try '20 + 31   - 9' 42
try '  1 + 2 + 4 - 2' 5

try '10 * 9' 90
try '(10 - 5) + 4 * 8' 37
try '(10 - 5) * 8 / 2' 20
try '(2 + 3) * 5 + (6 / 2)' 28
try '(2 + 3 * 4 - 1) * (2 + 4 - 1)' 65

echo OK
