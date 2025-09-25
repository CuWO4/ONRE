# ONRE

单头文件的零成本抽象的 O(n) 正则引擎.

---

## 核心特征

- 严格 O(n) 的匹配复杂度 (n为待匹配串长度);
- 在编译期将常量字符串正则表达式构建为自动机, 无运行时开销;
- 单头文件, 仅需 `include` 即可使用;
- 使用 Brzozowski 导数进行基于类型体操的函数式元编程, 很酷!
- 只支持标准正则表达式(即连接, 并, 闭包, 括号), 字符集为 `[0-9a-zA-Z]`. 不支持零宽断言; 不支持捕获组; 不支持量词扩展; 不支持部分匹配; 不支持反向引用.

## 效果

TODO

## 理论基础

### Brzozowski 导数

对于正则表达式 $R$ 和 字符 $x$, 我们记正则表达式 $\frac{\partial R}{\partial x}$ 为 $R$ 对 $x$ 的 Brzozowski 导数 (下简称导数), 其表示的语言为

\[L(\frac{\partial R}{\partial x}) = \{w \in \Sigma^*: xw \in L(R)\}\]

, 即在接受 $x$ 后, 所有可以使 $R$ 完成匹配的串. 其递归定义为

\[\frac{\partial \empty}{\partial x} = \empty\]

\[\frac{\partial \epsilon}{\partial x} = \empty\]

\[\frac{\partial y}{\partial x} =\begin{cases}
\epsilon,\qquad y = x \\
\empty,\qquad \text{otherwise}
\end{cases} \]

\[\frac{\partial (R|S)}{\partial x} = \frac{\partial R}{\partial x} | \frac{\partial S}{\partial x}\]

\[\frac{\partial (RS)}{\partial x} = \frac{\partial R}{\partial x} S | \delta(R) \frac{\partial S}{\partial x}\]

\[\frac{\partial (R^*)}{\partial x} = \frac{\partial R}{\partial x} R^*\]

, 其中 $\delta(\cdot)$ 为可空函数, 其定义为

\[\delta(R) = \begin{cases}
\epsilon,\qquad \epsilon \in L(R) \\
\empty,\qquad \text{otherwise}
\end{cases}\]

, 其递归定义为

\[\delta(\empty) = \empty\]

\[\delta(\epsilon) = \epsilon\]

\[\delta(x) = \empty\]

\[\delta(R|S) = \delta(R)|\delta(S)\]

\[\delta(RS) = \delta(R)\delta(S)\]

\[\delta(R^*) = \epsilon\]

. 基于此, 我们可以构建表达式 $R$ 的自动机 $M = (Q, \Sigma, \delta, q_0, Q_{\text{accept}})$, 其中

\[Q \subset \text{RE}\]

\[\delta(R, x) = \frac{\partial R}{\partial x}\]

\[q_0 = R\]

\[Q_\text{accept} = \{R \in RE: \epsilon \in L(R)\}\]

. 我们递归地对正则表达式求导, 直至没有新正则表达式生成, 即完成构建.

## 依赖

```text
Ubuntu clang version 18.1.8 (++20240731024944+3b5b5c1ec4a3-1~exp1~20240731145000.144)

--std=c++23
```
