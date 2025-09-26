# ONRE

A single-header, zero-overhead **O(n)** regular expression engine.

---

> This document is also available in other languages:
> - [简体中文](README.zh-CN.md)

---

## Key Features

- Strict **O(n)** matching complexity (where *n* is the length of the input string);
- Regular expressions written as constant strings are compiled into automata at compile time, with **no runtime overhead**;
- Single header file, just `#include` and use;
- Built on **Brzozowski derivatives**, realized with type-level functional metaprogramming — very cool!
- Supports all visible ASCII characters as alphabet;
- Supports standard regular expressions (concatenation, alternation, Kleene star, parentheses), `+` and `?` sugar, full-match `.`, character classes like `[a-z012]`, and character escapes;
- Not supported: zero-width assertions, capturing groups, extended quantifiers, partial matches, or back references.

## Performance Showcase

```log
=== Long String Tests (O(n) performance) ===
pattern: a*                                       pattern_len: 2     str: aaaaaaaa....aaaaaaaa      str_len: 100000   result: true   expected: true   time: 149us
pattern: a*b                                      pattern_len: 3     str: aaaaaaaa....aaaaaaab      str_len: 100001   result: true   expected: true   time: 144us
pattern: (ab)*                                    pattern_len: 5     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 143us
pattern: (a?b)+                                   pattern_len: 6     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 144us
pattern: [a-z]+                                   pattern_len: 6     str: mmmmmmmm....mmmmmmmm      str_len: 100000   result: true   expected: true   time: 144us
pattern: [a-z0-9]+                                pattern_len: 9     str: a1c3e5g7....w3y5a7c9      str_len: 100000   result: true   expected: true   time: 143us

=== Backtracking Killer Tests ===
pattern: (a|b)*c                                  pattern_len: 7     str: aaaaaaaa....aaaaaaaa      str_len: 10000    result: false  expected: false  time: 14us
pattern: (a|b)*a(a|b)*a                           pattern_len: 14    str: bbbbbbbb....bbbbbbba      str_len: 10001    result: false  expected: false  time: 14us
pattern: (a*)*                                    pattern_len: 5     str: aaaaaaaa....aaaaaaaa      str_len: 10000    result: true   expected: true   time: 14us
pattern: (a*(b*)*)*                               pattern_len: 10    str: bbbbbbbb....bbbbbbbb      str_len: 10000    result: true   expected: true   time: 15us
pattern: (a|b)*a(a|b)*b                           pattern_len: 14    str: aaaaaaaa....bbbbbbbb      str_len: 20001    result: false  expected: false  time: 17us
pattern: ([a-z])*z                                pattern_len: 9     str: xxxxxxxx....xxxxxxxx      str_len: 10000    result: false  expected: false  time: 14us

=== Long String Complex Pattern Tests ===
pattern: (a|b)*                                   pattern_len: 6     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 149us
pattern: (a*b*)*                                  pattern_len: 7     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 149us
pattern: a*ba*                                    pattern_len: 5     str: aaaaaaaa....aaaaaaaa      str_len: 100001   result: true   expected: true   time: 144us
pattern: a*ca*                                    pattern_len: 5     str: aaaaaaaa....aaaaaaaa      str_len: 100001   result: false  expected: false  time: 72us
pattern: start(x)*end                             pattern_len: 12    str: startxxx....xxxxxend      str_len: 100008   result: true   expected: true   time: 145us
pattern: start(y)*end                             pattern_len: 12    str: startxxx....xxxxxend      str_len: 100008   result: false  expected: false  time: 0us
pattern: prefix(x)+(y)?suffix                     pattern_len: 20    str: prefixxx....xxsuffix      str_len: 100012   result: true   expected: true   time: 145us
pattern: prefix(x)+(y)?suffix                     pattern_len: 20    str: prefixxsuffix             str_len: 13       result: true   expected: true   time: 0us
pattern: prefix(x)+(y)?suffix                     pattern_len: 20    str: prefixsuffix              str_len: 12       result: false  expected: false  time: 0us
pattern: (a+b)+                                   pattern_len: 6     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 144us
pattern: (a?b?)+                                  pattern_len: 7     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 149us
pattern: (ab)*                                    pattern_len: 5     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 150us
pattern: (a|b)*                                   pattern_len: 6     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 166us
pattern: (aa)*                                    pattern_len: 5     str: abababab....abababab      str_len: 100000   result: false  expected: false  time: 0us
pattern: .*                                       pattern_len: 2     str: abababab....abababab      str_len: 100000   result: true   expected: true   time: 149us

=== Divisible Test (match all binary number mod 5 remain 2) ===
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000000                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000001                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000010                    str_len: 6        result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000011                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000100                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000101                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000110                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 000111                    str_len: 6        result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001000                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001001                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001010                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001011                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001100                    str_len: 6        result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001101                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001110                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 001111                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 010000                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 010001                    str_len: 6        result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 010010                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 010011                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 010100                    str_len: 6        result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11010111....01100111      str_len: 31       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11001001....11000110      str_len: 30       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11001000....01101001      str_len: 31       result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11001100....01110011      str_len: 31       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11101001....01010001      str_len: 31       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11001010....11111111      str_len: 29       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 10101011....01001010      str_len: 30       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 11000100....11101100      str_len: 31       result: true   expected: true   time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 10001110....00101001      str_len: 30       result: false  expected: false  time: 0us
pattern: ((((0*1)0)((10*....*1)0)((10*1)0)*       pattern_len: 321   str: 10001101....11001101      str_len: 31       result: false  expected: false  time: 0us
```

As shown, for extremely long strings, catastrophic backtracking cases, oversized or highly complex patterns, the engine consistently achieves **O(n)** performance. In contrast, backtracking-based engines will almost certainly choke on such inputs (see the well-known [Cloudflare incident](https://www.reddit.com/r/sysadmin/comments/c8eymj/cloudflare_outage_caused_by_deploying_bad_regular/)).

Compile times for these complex patterns remain fully controlled and acceptable:

```sh
> time make -j20
...
real    0m8.090s
user    0m29.398s
sys     0m2.264s
```

## Usage

```cpp
#include "regex.hpp"

void f() {
  bool result = onre::Regex<"((ab)*|c*|b)(@\\.)?">::match("abab"); // true
}
```

## Theoretical Foundation

### Brzozowski Derivatives

For a regular expression $R$ and a character $x$, the Brzozowski derivative $\frac{\partial R}{\partial x}$ (hereafter simply “derivative”) is defined as the language

$$L(\frac{\partial R}{\partial x}) = \lbrace w \in \Sigma^*: xw \in L(R)\rbrace$$

i.e., after consuming $x$, the set of all strings $w$ that would still make $R$ accept.

The recursive definition is:

$$\frac{\partial \emptyset}{\partial x} = \emptyset$$

$$\frac{\partial \epsilon}{\partial x} = \emptyset$$

$$\frac{\partial y}{\partial x} =\begin{cases}
\epsilon,\qquad y = x \\
\emptyset,\qquad \text{otherwise}
\end{cases} $$

$$\frac{\partial (R|S)}{\partial x} = \frac{\partial R}{\partial x} | \frac{\partial S}{\partial x}$$

$$\frac{\partial (RS)}{\partial x} = \frac{\partial R}{\partial x} S | \delta(R) \frac{\partial S}{\partial x}$$

$$\frac{\partial (R^{\ast})}{\partial x} = \frac{\partial R}{\partial x} R^{\ast}$$

Here $\delta(\cdot)$ is the nullability function:

$$\delta(R) = \begin{cases}
\epsilon,\qquad \epsilon \in L(R) \\
\emptyset,\qquad \text{otherwise}
\end{cases}$$

with recursive definition:

$$\delta(\emptyset) = \emptyset$$

$$\delta(\epsilon) = \epsilon$$

$$\delta(x) = \emptyset$$

$$\delta(R|S) = \delta(R)|\delta(S)$$

$$\delta(RS) = \delta(R)\delta(S)$$

$$\delta(R^*) = \epsilon$$

Based on this, we construct the automaton $M = (Q, \Sigma, \delta, q_0, Q_{\text{accept}})$ where

$$Q \subset \text{RE}$$

$$\delta(R, x) = \frac{\partial R}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace R \in RE: \epsilon \in L(R)\rbrace$$

We recursively compute derivatives of the expression until no new expressions are generated, thereby completing the construction.

## Testing

```sh
make test -j10
```

## Dependencies

```text
Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)

--std=c++23
```

## TODO

- [ ] Add Hopcroft DFA minimization (In fact, the automaton generated by Brzozowski is a minimal automaton when it can perform equivalence checks on subexpressions; in practice, when only comparing regular expression literals, the redundancy of the generated automaton is highly correlated with the ability to algebraically simplify and normalize regular expressions. So in theory, as long as the regular expression is sufficiently simplified, the generated automaton is almost always a minimal automaton.).
