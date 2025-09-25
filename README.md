# ONRE

单头文件的零成本抽象的 O(n) 正则引擎.

---

## 核心特征

- 严格 O(n) 的匹配复杂度 (n为待匹配串长度);
- 在编译期将常量字符串正则表达式构建为自动机, 无运行时开销;
- 单头文件, 仅需 `include` 即可使用;
- 使用 Brzozowski 导数进行基于类型体操的函数式元编程, 很酷!
- 只支持标准正则表达式(即连接, 并, 闭包, 括号), 字符集为 `[0-9a-zA-Z]`. 不支持零宽断言; 不支持捕获组; 不支持量词扩展; 不支持部分匹配; 不支持反向引用.

## 用法

```cpp
#include "regex.hpp"

void f() {
  bool result = onre::Regex<"(ab)*|c*|b">::match("abab"); // true
}
```

## 效果

```log
pattern: a                        str: a                        str_len: 1       result: true  expected: true  time: 0us
pattern: a                        str: b                        str_len: 1       result: false expected: false time: 0us
pattern: a|b                      str: a                        str_len: 1       result: true  expected: true  time: 0us
pattern: a|b                      str: b                        str_len: 1       result: true  expected: true  time: 0us
pattern: a|b                      str: c                        str_len: 1       result: false expected: false time: 0us
pattern: a*                       str:                          str_len: 0       result: true  expected: true  time: 0us
pattern: a*                       str: a                        str_len: 1       result: true  expected: true  time: 0us
pattern: a*                       str: aa                       str_len: 2       result: true  expected: true  time: 0us
pattern: a*                       str: b                        str_len: 1       result: false expected: false time: 0us
pattern: ab                       str: ab                       str_len: 2       result: true  expected: true  time: 0us
pattern: ab                       str: a                        str_len: 1       result: false expected: false time: 0us
pattern: (ab)*                    str: abab                     str_len: 4       result: true  expected: true  time: 0us
pattern: (ab)*                    str: aba                      str_len: 3       result: false expected: false time: 0us
pattern: a*b                      str: aaab                     str_len: 4       result: true  expected: true  time: 0us
pattern: a*b                      str: b                        str_len: 1       result: true  expected: true  time: 0us
pattern: a*b                      str: aaac                     str_len: 4       result: false expected: false time: 0us
pattern: (a|b)*|c                 str: abbac                    str_len: 5       result: false expected: false time: 0us
pattern: (a|b)*|c                 str: c                        str_len: 1       result: true  expected: true  time: 0us
pattern: (a|b)*|c                 str: d                        str_len: 1       result: false expected: false time: 0us

=== Boundary Tests ===
pattern:                          str:                          str_len: 0       result: true  expected: true  time: 0us
pattern:                          str: a                        str_len: 1       result: false expected: false time: 0us
pattern: a                        str:                          str_len: 0       result: false expected: false time: 0us
pattern: a*                       str:                          str_len: 0       result: true  expected: true  time: 0us
pattern: (a|)                     str: a                        str_len: 1       result: true  expected: true  time: 0us
pattern: (a|)                     str:                          str_len: 0       result: true  expected: true  time: 0us

=== Long String Tests (O(n) performance) ===
pattern: a*                       str: aaaaaaaaaaaaaaaaaaaa...  str_len: 100000  result: true  expected: true  time: 223us
pattern: a*b                      str: aaaaaaaaaaaaaaaaaaaa...  str_len: 100001  result: true  expected: true  time: 223us
pattern: (ab)*                    str: abababababababababab...  str_len: 100000  result: true  expected: true  time: 223us

=== Backtracking Killer Tests ===
pattern: (a|b)*c                  str: aaaaaaaaaaaaaaaaaaaa...  str_len: 10000   result: false expected: false time: 22us
pattern: (a|b)*a(a|b)*a           str: bbbbbbbbbbbbbbbbbbbb...  str_len: 10001   result: false expected: false time: 22us
pattern: (a*)*                    str: aaaaaaaaaaaaaaaaaaaa...  str_len: 10000   result: true  expected: true  time: 22us
pattern: (a*(b*)*)*               str: bbbbbbbbbbbbbbbbbbbb...  str_len: 10000   result: true  expected: true  time: 22us
pattern: (a|b)*a(a|b)*b           str: aaaaaaaaaaaaaaaaaaaa...  str_len: 20001   result: false expected: false time: 28us

=== Mixed Character Tests ===
pattern: (a|b|c|d|e)*             str: abcdefghijklmnopqrst...  str_len: 10000   result: false expected: false time: 0us

=== Non-matching Long String Tests ===
pattern: (a|b|c|d|e)*             str: 11111111111111111111...  str_len: 10000   result: false expected: false time: 0us
pattern: a*                       str: aaaaaaaaaaaaaaaaaaaa...  str_len: 10000   result: false expected: false time: 11us
```

可以看到, 在超长串和回溯地狱用例下, 引擎仍然十分稳定地取得了 O(n) 的结果, 同样的用例对于回溯引擎则几乎必然崩溃 (例如著名的 [Cloudflare 事件](https://www.reddit.com/r/sysadmin/comments/c8eymj/cloudflare_outage_caused_by_deploying_bad_regular/)).

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

## 依赖

```text
Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)

--std=c++23
```

## TODO

- [ ] 引入 Hopcroft DFA 化简 (尽管 Brzozowski 导数构造法几乎总是接近最小 DFA).
