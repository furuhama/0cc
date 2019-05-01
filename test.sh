#!/bin/bash

try() {
  input="$1"
  expected="$2"

  ./0cc "$input" > tmp.s
  gcc-8 tmp.s -o tmp
  ./tmp
  actual="$?"

  if [ "$actual" != "$expected" ]; then
    echo -e "[line $BASH_LINENO] expected: $expected\tinput: '$input'"
    echo "but got:  $actual"
    exit 1
  fi
}

try '0;' 0
try '42;' 42

try '5-5+10-10;' 0
try '20+31-9;' 42

try '20 + 31   - 9   ;' 42
try '  1 + 2 + 4 - 2  ;' 5

try '10 * 9;' 90
try '(10 - 5) + 4 * 8;' 37
try '(10 - 5) * 8 / 2;' 20
try '(2 + 3) * 5 + (6 / 2);' 28
try '(2 + 3 * 4 - 1) * (2 + 4 - 1);' 65
try '(2 + 3 * 4 - 12 + 5 - 7) * (2 + 4 - 1) * (100 + 23 - 23 + 73 - 42 + 2379 + 10 * 20 - 1) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7) * (2 + 3 * 4 - 12 + 5 - 7);' 0
try '-3 + 5 * 2;' 7
try '-3 * +5 + 20;' 5

try '1 == 2;' 0
try '1 == 1;' 1
try 'a = 1 == 1; return a;' 1
try 'a = 1; b = 2; return a == b;' 0
try '1 != 2;' 1
try '3 != 3;' 0
try 'a = 1 != 2; b = a != 0; return b;' 1
try 'a = 1; b = 2; c = a == a != b; return c;' 1
try '1 <= 2;' 1
try '1 < 2;' 1
try '2 <= 2;' 1
try '2 < 2;' 0
try '3 <= 2;' 0
try '3 < 2;' 0
try '1 >= 2;' 0
try '1 > 2;' 0
try '2 >= 2;' 1
try '2 > 2;' 0
try '3 >= 2;' 1
try '3 > 2;' 1
try '-2 < -3;' 0
try 'a = 2 < 3; return a;' 1
try 'a = 1; b = a + 1 < 3; return b;' 1
try 'a = 1; b = a + (1 < 3); return b;' 2

try '42; 0;' 0
try '42; 0; 30;' 30

try 'a = 42; a;' 42
try 'd = 0; d;' 0
try 'z = 0; -z;' 0
try 'c = -5; -c;' 5
try 'h = -10; i = +10; h + i + 42;' 42
try 'a = 42; 18;' 18
try 'a = 8; b = 4; a;' 8
try 'z = 100; x = 20; y = 2; z;' 100
try 'w = 5; v = 2; w + v;' 7
try '(a = 2); a;' 2
try '(a = 2); b = (a + 10 * 5); b;' 52
try 'a = b = 2; a + b;' 4
try 'a = b = 3 ; 1 + 1 + a + b  ;' 8

try 'return 1;' 1
try '100; return 10; 50;' 10
try 'a = 10; return a;' 10
try 'b = 42; c = 8; return c; b;' 8
try 'v = 20; x = 10; return v + x; v;' 30
try 'v = 20; x = 10; return v + v + x; v;' 50
try '(a = b = 4); c = 1; return a + b + c;' 9
try 'e = +5; w = -10; (a = b = -1); return a * b * e - w;' 15

try 'abc = 10; return abc;' 10
try 'abc = 10; def = 20; return abc + def;' 30
try 'xyz = abc = 15; pqr = xyz + abc; return xyz + pqr;' 45
try 'abcd = 10; abc = 5; ab = 2; a = -1; return abcd + abc * a + ab;' 7
try 'abcd = 10; abc = 5; ab = 2; a = -1; return abcd + abc * a + ab >= 3;' 1

echo OK
