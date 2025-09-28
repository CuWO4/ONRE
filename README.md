# ONRE

单头文件的零成本抽象的 O(n) 正则引擎.

---

## 核心特征

- 严格 O(n) 的匹配复杂度 (n为待匹配串长度);
- 在编译期将常量字符串正则表达式构建为自动机, 无运行时开销;
- 单头文件, 仅需 `include` 即可使用;
- 使用 Brzozowski 导数进行基于类型体操的函数式元编程, 很酷!
- 支持所有可见 ASCII 字符作为字母表;
- 支持标准正则表达式(即连接, 并, 闭包, 括号), 支持 `+`, `?` 语法糖, 支持全匹配 `.`, 支持字符类 (形如 `[a-z012]`), 支持字符转义;
- 不支持零宽断言; 不支持捕获组; 不支持量词扩展; 不支持部分匹配; 不支持反向引用.

## 性能展示

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

可以看到, 在超长串, 回溯地狱, 超长模式, 超复杂模式用例下, 引擎仍然十分稳定地取得了 O(n) 的结果, 同样的用例对于回溯引擎则几乎必然崩溃 (例如著名的 [Cloudflare 事件](https://www.reddit.com/r/sysadmin/comments/c8eymj/cloudflare_outage_caused_by_deploying_bad_regular/)).

编译这些复杂模式的时间同样完全是完全可控且可接受的:

```sh
> time make -j20
...
real    0m8.090s
user    0m29.398s
sys     0m2.264s
```

## 用法

```cpp
#include "regex.hpp"

void f() {
  bool result = onre::Regex<"((ab)*|c*|b)(@\\.)?">::match("abab"); // true
}
```

## 理论基础

### Brzozowski 导数

对于正则表达式 $R$ 和 字符 $x$, 我们记正则表达式 $\frac{\partial R}{\partial x}$ 为 $R$ 对 $x$ 的 Brzozowski 导数 (下简称导数), 其表示的语言为

$$L(\frac{\partial R}{\partial x}) = \lbrace w \in \Sigma^*: xw \in L(R)\rbrace$$

, 即在接受 $x$ 后, 所有可以使 $R$ 完成匹配的串. 其递归定义为

$$\frac{\partial \emptyset}{\partial x} = \emptyset$$

$$\frac{\partial \epsilon}{\partial x} = \emptyset$$

$$\frac{\partial y}{\partial x} =\begin{cases}
\epsilon,\qquad y = x \\
\emptyset,\qquad \text{otherwise}
\end{cases} $$

$$\frac{\partial (R|S)}{\partial x} = \frac{\partial R}{\partial x} | \frac{\partial S}{\partial x}$$

$$\frac{\partial (RS)}{\partial x} = \frac{\partial R}{\partial x} S | \delta(R) \frac{\partial S}{\partial x}$$

$$\frac{\partial (R^{\ast})}{\partial x} = \frac{\partial R}{\partial x} R^{\ast}$$

, 其中 $\delta(\cdot)$ 为可空函数, 其定义为

$$\delta(R) = \begin{cases}
\epsilon,\qquad \epsilon \in L(R) \\
\emptyset,\qquad \text{otherwise}
\end{cases}$$

, 其递归定义为

$$\delta(\emptyset) = \emptyset$$

$$\delta(\epsilon) = \epsilon$$

$$\delta(x) = \emptyset$$

$$\delta(R|S) = \delta(R)|\delta(S)$$

$$\delta(RS) = \delta(R)\delta(S)$$

$$\delta(R^*) = \epsilon$$

. 基于此, 我们可以构建表达式 $R$ 的自动机 $M = (Q, \Sigma, \delta, q_0, Q_{\text{accept}})$, 其中

$$Q \subset \text{RE}$$

$$\delta(R, x) = \frac{\partial R}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace R \in RE: \epsilon \in L(R)\rbrace$$

. 我们递归地对正则表达式求导, 直至没有新正则表达式生成, 即完成构建.

## 测试

```sh
make test -j10
```

## 依赖

```text
Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)

--std=c++23
```

## TODO

- [ ] 引入 Hopcroft DFA 化简 (事实上, Brzozowski 生成的自动机在能够对子表达式进行等价性检测时, 是最小自动机; 只比较正则表达式字面的实践中, 生成的自动机冗余度则与对正则表达式代数化简和规范化的能力高度相关. 所以理论上, 只要充分化简正则表达式, 则生成的自动机几乎总是最小自动机).
