[![en](https://img.shields.io/badge/lang-en-green)](README.md)
[![zh-cn](https://img.shields.io/badge/lang-%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87-green)](README.zh-cn.md)

# ONRE

单头文件的零成本抽象的 $O(n)$ 正则引擎.

---

## ✨ 核心特征

- ✔️ 严格 $O(|s|)$ 的匹配复杂度, $|s|$ 为待匹配串长度;
- ✔️ 在编译期将常量字符串正则表达式构建为自动机, 无运行时开销;
- ✔️ 单头文件, 仅需 `include` 即可使用;
- ✔️ **支持捕获组**以及基于捕获组的替换;
- ✔️ 使用 Brzozowski 导数进行基于类型体操的函数式元编程, 确保编译迅速且最终结果几乎总是最简自动机, 而且很酷!
- ✔️ 支持所有可见 ASCII 字符作为字母表;
- ✔️ 支持标准正则表达式(连接, 并 `|`, 闭包 `*`, 括号 `()`); 支持 `+`, `?`; 支持通配 `.`; 支持字符类 (形如 `[^a-z012]`); 支持字符转义 (`\n`, `\t`, `\d`, `\s`, `\w`, `\x[HEX]`, `\[`, `\]`, `\*`...); 支持量词 (`{n}`, `{n,}`, `{,m}`, `{n,m}`);
- ❌️ 不支持零宽断言;
- ❌️ 不支持反向引用;
- ❌️ 捕获消歧规则不符合 POSIX 标准或 Perl 标准.

## ⏱️ 性能展示

```text
=== Match ===
pattern: a*                        pattern_len: 2    str: aaaaaaaa....aaaaaaaa  str_len: 100000  time: 127us
pattern: a*b                       pattern_len: 3    str: aaaaaaaa....aaaaaaab  str_len: 100001  time: 127us
pattern: (ab)*                     pattern_len: 5    str: abababab....abababab  str_len: 100000  time: 134us
pattern: (a?b)+                    pattern_len: 6    str: abababab....abababab  str_len: 100000  time: 128us
pattern: [a-z]+                    pattern_len: 6    str: mmmmmmmm....mmmmmmmm  str_len: 100000  time: 131us
pattern: [a-z0-9]+                 pattern_len: 9    str: a1c3e5g7....w3y5a7c9  str_len: 100000  time: 127us
pattern: (a|b)*c                   pattern_len: 7    str: aaaaaaaa....aaaaaaaa  str_len: 10000   time: 12us
pattern: (a|b)*a(a|b)*a            pattern_len: 14   str: bbbbbbbb....bbbbbbba  str_len: 10001   time: 13us
pattern: (a*)*                     pattern_len: 5    str: aaaaaaaa....aaaaaaaa  str_len: 10000   time: 12us
pattern: (a*(b*)*)*                pattern_len: 10   str: bbbbbbbb....bbbbbbbb  str_len: 10000   time: 12us
pattern: (a|b)*a(a|b)*b            pattern_len: 14   str: aaaaaaaa....bbbbbbbb  str_len: 20001   time: 12us
pattern: ([a-z])*z                 pattern_len: 9    str: xxxxxxxx....xxxxxxxx  str_len: 10000   time: 12us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000000                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000001                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000010                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000011                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000100                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000101                str_len: 6       time: 0us
pattern: ((((0*1)0)....((10*1)0)*  pattern_len: 321  str: 000110                str_len: 6       time: 0us

=== Replace ===
pattern: a+Xb+         pattern_len: 5   replace_rule: $0mid$0  str: aaaaaaa....bbbbbbb  str_len: 200001  time: 952us
pattern: (x*)end       pattern_len: 7   replace_rule: $1-END   str: xxxxxxx....xxxxend  str_len: 10003   time: 75us
pattern: start(x*)end  pattern_len: 12  replace_rule: $1       str: startxx....xxxxend  str_len: 10008   time: 78us
pattern: (a|b)+        pattern_len: 6   replace_rule: $0       str: aaaaaaa....aaaaaaa  str_len: 300000  time: 2892us
pattern: (a+)+b        pattern_len: 6   replace_rule: $0       str: aaaaaaa....aaaaaab  str_len: 10001   time: 62us
pattern: (a?)+b        pattern_len: 6   replace_rule: $0       str: aaaaaaa....aaaaaab  str_len: 10001   time: 272us
pattern: (a|b)+c       pattern_len: 7   replace_rule: $0       str: aaaaaaa....aaaaaac  str_len: 10001   time: 161us
pattern: (ab|a)*c      pattern_len: 8   replace_rule: $0       str: aaaaaaa....aaaaaac  str_len: 10001   time: 167us
pattern: (a+)(a+)b     pattern_len: 9   replace_rule: $1|$2    str: aaaaaaa....aaaaaab  str_len: 10001   time: 336us
pattern: (a|ab)+b      pattern_len: 8   replace_rule: OK       str: aaaaaaa....aaaaaab  str_len: 10001   time: 157us
pattern: (a*)b         pattern_len: 5   replace_rule: $1B      str: aaaaaaa....aaaaaab  str_len: 10001   time: 61us
```

可以看到, 在超长串, 回溯地狱, 超长模式, 超复杂模式用例下, 引擎仍然十分稳定地取得了 O(n) 的结果, 同样的用例对于回溯引擎则几乎必然崩溃 (例如著名的 [Cloudflare 事件](https://www.reddit.com/r/sysadmin/comments/c8eymj/cloudflare_outage_caused_by_deploying_bad_regular/)).

编译这些(总数远超过上面展示的, 详见 `tests/`)复杂模式的时间是完全可控且可接受的:

```sh
> make clean && time make -j30
real    0m13.845s
user    1m19.445s
sys     0m3.850s
```

## 🤔 用法

```cpp
#include "onre.hpp"
#include <string>

void f() {
  bool result = onre::Match<"((ab)*|c*|b)(@\\.)?">::eval("abab");
  std::string replaced = onre::Replace<"ab(.*)ab">::eval("$0, $1, $$", "ababab");
  // result = true; replaced = "ababab, ab, $"
}
```

替换规则中, 使用 `$N` 来引用第 $N$ 个捕获组 (按照捕获组左括号位置排序, 从 1 开始). `$0` 为串本身. 通过 `$$` 来表示 `$`.

若 `onre::Replace` 无法匹配, 则 `eval()`结果是未定义的; 如果替换规则不合法, 则 `eval()` 将会返回空串. 因此, 对于有可能匹配失败的场景, 应该先使用 `onre::Match` 检查是否匹配.

在代码的任何位置实例化 `onre::Match` 和 `onre::Replace`, 都会导致编译期展开, 增加编译时间, 即使动态运行时永远不可能运行到. 同一个编译单元中, 相同 pattern 的匹配器只会被实例化一次, 不同编译单元中的匹配器则在每个单元的编译中都会被实例化, 因此把复杂模式的匹配抽象到一个编译单元中会显著降低编译用时.

```cpp
#include "onre.hpp"

bool is_valid_email(std::string email) {
  // only compiled once
  return onre::Match<"[-a-zA-Z0-9.]+@([-a-zA-Z0-9]+.)+[-a-zA-Z0-9]+">::eval(email);
}
```

对于有复杂的表达式的程序, 编译选项应该加上 `-fbracket-depth=[A BIG NUMBER] -ftemplate-depth=[A BIG NUMBER]` 来允许编译期进行更深的编译期递归.

## ⚖️ 标准

与 POSIX (IEEE Std 1003.1-2001) 最左最长 (Leftmost-Longest) 标准不同, ONRE 遵循最右最长匹配. 具体来说, 闭包内的捕获组将匹配最后一个可以完成匹配的串. 例如使用 `((a*)(a*)b)*` 匹配 `aaababaaaab`, 有结果:

| 捕获组 | POSIX | ONRE |
| - | - | - |
| `$1` (`((a*)(a*)b)`) | `aaab` | `aaaab` |
| `$2` (第一个 `(a*)`) | `aaa` | `aaaa` |
| `$3` (第二个 `(a*)`) | | |

相当于

```text
 .-- $1 --.
 |        |
 +-$2--.  |
 |     |  |
 V     V  V                         POSIX
 a  a  a  b  a  b  a  a  a  a  b  ---------
                   ^        ^  ^    ONRE
                   |        |  |
                   +-- $2 --`  |
                   |           |
                   `-----$1----`
```

其他例子:

```cpp
onre::Replace<"((a*)b)*">::eval("$2", "aabb") => "" // aab-()-b
onre::Replace<"(a|ab)+b">::eval("$1", "abab") => "a" // ab-(a)-b
```

## 🤯 理论基础

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

. 基于此, 我们可以构建表达式 $R$ 的确定有穷自动机 $M = (Q, \Sigma, \delta, q_0, Q_{\text{accept}})$, 其中

$$Q \subset \text{RE}$$

$$\delta(R, x) = \frac{\partial R}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace R \in RE: \epsilon \in L(R)\rbrace$$

. 我们递归地对正则表达式求导, 直至没有新正则表达式生成, 即完成构建.

### 动作代数

我们考虑存在 $N$ 个槽 $s_0, s_1, s_2, ..., s_{N-1}$, 每个槽可以储存一个自然数或 -1 (表示未被初始化过). 我们记录将第 $i$ 个槽设置为 $x$ 为 $\text{set}(i, x)$. 如果有两个动作, 我们定义两个动作 $\alpha, \beta$ 的连接 $\alpha \cdot \beta$ 为先做 $\alpha$ 中的动作, 再做 $\beta$ 中的动作. 我们再定义 $\omega$ 为空动作, 表示什么都不做, 且 $\alpha \cdot \omega = \omega \cdot \alpha = \alpha$. 我们把这样的结构称为动作代数.

形式化地, 我们称一个槽格局为 $N$ 元组 $(s_0, s_1, ..., s_{N-1}), s_i \in \mathbb N \cup \lbrace -1 \rbrace$, 把 $N$ 元槽格局集合记作 $S_N = (\mathbb N \cup \lbrace -1 \rbrace)^N$.

若一个集合 $A_N \subset S_N\,^{S_N}$ 满足:

1. $\omega \in A, \text{where } \omega(S) = S$;
1. $\text{set}(i, x) \in A, \forall i \in \lbrace 0, 1, ..., N-1 \rbrace, x \in \mathbb N, \text{where } \text{set}(i, x)(s_0, s_1, ..., s_i, ..., s_{N-1}) = (s_0, s_1, ..., x, ..., s_{N-1})$;
1. $\alpha \cdot \beta \in A, \forall \alpha, \beta \in A, \text{where } \alpha \cdot \beta (T) = \beta(\alpha(T)) $,

那么我们称 $A_N$ 为动作集.

### 扩展正则表达式

如果我们允许正则表达式中有符号 `<i>`, 表示一个零宽 (不消耗字符) 匹配, 并且在匹配时, 执行动作 $\text{set}(i, p)$, 其中 $p$ 为当前已消耗字符的数量 (即当前"光标"在待匹配串中的位置的索引), 那么我们把这样的正则表达式称为扩展正则表达式.

形式化地, 给定自然数 $N$ 和字母表 $\Sigma$, 如果集合 $E$ 满足:

1. $\emptyset \in E$;
1. $\epsilon \in E$;
1. $ \langle i \rangle \in E, \forall i \in \lbrace 0, 1, 2, ..., N-1 \rbrace $;
1. $a \in E, \forall a \in \Sigma$;
1. $R|S \in E, \forall R, S \in E$;
1. $RS \in E, \forall R, S \in E$;
1. $R^\ast \in E, \forall R$,

那么我们称 $E$ 为字母表 $\Sigma$ 上带有 $N$ 个槽的扩展正则表达式.

每个扩展正则表达式定义一个语言, $L(\langle i\rangle) = \lbrace \epsilon \rbrace$, 其他规则与标准正则表达式一致.

我们在扩展正则表达式开始匹配时, 置初始槽状态为 $(-1, -1, ..., -1)$. 匹配时, 每一个子表达式完成匹配时, 执行其动作, 那么在完成匹配时, 将产生一个槽状态, 称为这个正则表达式对这串的输出. 输出不是唯一的 (存在多种可能的路径), 我们不考虑消除歧义的问题.

例如对于 POSIX 正则表达式 `a(a*)a`, 它尝试捕获去除首字母和尾字母的串, 它可以看作扩展正则表达式 $\langle 0\rangle a^\ast\langle 1\rangle a$, 那么在完成匹配时, 将给出输出 $(s_0, s_1)$, 即为捕获组在待匹配串的起止索引(左开右闭).

### $v$ 记号

对于扩展正则表达式 $R$, $v(R)$ 给出当 $R$ 接受空串时可能的动作集合, 若 $R$ 不接受空串, 则为空. $v$ 可以递归定义:

$$ v(\emptyset) = \emptyset $$

$$ v(\epsilon) = \lbrace \omega \rbrace $$

$$ v(\langle i\rangle) = \lbrace \text{set}(i, p) \rbrace $$

$$ v(a) = \emptyset $$

$$ v(R|S) = v(R) \cup v(S) $$

$$ v(RS) = \lbrace \alpha \cdot \beta : \alpha \in v(R), \beta \in v(S) \rbrace$$

$$ v(R^\ast) = \lbrace \omega \rbrace $$

### 扩展 Brzozowski 导数

对于扩展正则表达式 $R$ 和字符 $x$, 我们称 $\frac{\partial R}{\partial x}$ 为扩展 Brzozowski 导数. 扩展 Brzozowski 导数, 给出所有可能二元对 $(S, \alpha)$ 的集合, 其中 $(S, \alpha)$ 表示 $R$ 可以执行动作 $\alpha$, 然后消耗字符 $x$, 然后使用 $S$ 完成匹配. 我们可以递归定义:

$$\frac{\partial \emptyset}{\partial x} = \emptyset$$

$$\frac{\partial \epsilon}{\partial x} = \emptyset$$

$$\frac{\partial \langle i\rangle}{\partial x} = \emptyset$$

$$\frac{\partial y}{\partial x} =\begin{cases}
\lbrace (\epsilon, \omega) \rbrace,\qquad y = x \\
\emptyset,\qquad \text{otherwise}
\end{cases} $$

$$\frac{\partial (R|S)}{\partial x} = \frac{\partial R}{\partial x} \cup \frac{\partial S}{\partial x}$$

$$\frac{\partial (RS)}{\partial x} = \lbrace (R'S, \alpha) : (R', \alpha) \in \frac{\partial R}{\partial x} \rbrace \cup \lbrace (S', \alpha \cdot \beta) : \alpha \in v(R), (S', \beta) \in \frac{\partial S}{\partial x}\rbrace $$

$$\frac{\partial (R^{\ast})}{\partial x} = \lbrace (R' R^{\ast}, \alpha) : (R', \alpha) \in \frac{\partial R}{\partial x} \rbrace$$

### TNFA 与导数构造

在 NFA 的基础上, 我们为每一条边都分配一个动作, 在采取这个转移时, 先执行动作, 再消耗字符, 再转移到新状态, 那么这种自动机称为 Tagged Nondeterministic Finite Automata (TNFA). TNFA 的接收状态还会有一个接受动作, 当自动机在此处接受串时, 则执行该接受节点的接受动作.

形式化地，我们称 TNFA 是七元组 $(Q, \Sigma, N, \delta, q_0, Q_\text{accept}, A_\text{accept})$, 其中 $Q$ 是状态集, $\Sigma$ 是字符表, $N$ 是槽数, $\delta: Q \times \Sigma \rightarrow 2 ^ {Q \times A_N} $ 表示在某节点接受了某个字符后, 可到达的状态, 以及转移时的动作, $q_0$ 表示初始状态, $Q_\text{accept}$ 表示接受状态集合, $A_\text{accept} = Q_\text{accept} \rightarrow A_N$ 表示接受动作.

我们可以利用扩展 Brzozowski 来构造一个与扩展正则表达式 R 等价的 TNFA $M = (Q, \Sigma, N, \delta, q_0, Q_\text{accept}, A_\text{accept})$. 与 Brzozowski 构造 DFA 类似, 我们置

$$Q \subset \text{Extended RE}$$

$$N = \text{Number of Unique Slots in } R$$

$$\delta(q, x) = \frac{\partial q}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace q : \epsilon \in L(q) \rbrace$$

$$A_\text{accept}(q) = v(q)$$

对于 $A_\text{accept}$, 我们的构造有歧义 (可能有多种可能动作) 并不符合 TNFA 的定义, 但是我们先这样, 让接收状态有多种可选动作, 我们在下文中介绍如何从中选取一个.

### 活跃状态 TNFA 模拟捕获捕获组

回溯地模拟 TNFA 复杂度是指数的, 和回溯引擎没有区别(甚至潜在地更慢). 为了约束复杂度, 我们使用类活跃变量 NFA 模拟法的方法, 并引入一个启发式仲裁来解决歧义.

现在我们先考虑叙述, 给定 TNFA, 对于一个接受串, 沿着任何一条从起始状态到终止状态的路径, 得到的槽状态都是原扩展正则表达式的一个可能输出. 换而言之, 对于 `(a*)(a*)` (即 $\langle 0\rangle a^\ast\langle 1\rangle\langle 2\rangle a^\ast\langle 3\rangle$), 沿着不同路径存在多种可能的输出, 但是对于任何一个输出, 都可以保证 $\langle 0\rangle, \langle 1\rangle$ 分别标记了第一个 `a*` 的左右端点, $\langle 2\rangle, \langle 3\rangle$ 分别标记了第二个 `a*` 的左右端点, 且不会重叠. 我认为这是显然的, 但给出一个形式化的证明是困难的.

那么我们考虑算法: TNFA 有 $S$ 个结点, 分别标号为 $q_0, q_1, q_2, ..., q_{S-1}$, 其中 $q_0$ 为开始状态, $N$ 个槽. 那么我们置容器 `bool is_active[S]` 和 `number slots[S][N]`. `slots[i]` 是第 $i$ 个状态的槽格局.

开始时,

```text
is_active <- all false
is_active[0] <- true
slots[i][j] <- all -1
```

接下来, 消耗一个字符 `c`, 并

```text
bool new_active[S] <- all false
number new_slots[S][N] <- all -1
for i = 0, 1, 2, ..., S-1
  if ! is_active[i] continue
  for R', a in delta(q_i)
    dest_state = state idx of R'
    new_slots_row <- a(slots[i])
    if ! new_active[dest_state]
      new_slots[dest_state] = new_slots_row
    else
      new_slots[dest_state] = arbitration(new_slots[dest_state], new_slots_row)
    new_active[dest_state] = 1

if new_active all false
  failed to match

active <- new_active
slots <- new_slots
```

如果无字符可消耗, 则:

```text
number result_slots[N] = null
for q in Q
  if ! is_active[q] continue
  if ! is_accept(q) continue
  for a in A_accept(q)
    if result_slots == null
      result_slots = a(slots[q])
    else
      result_slots = arbitration(result_slots, a(slots[q]))

if result_slots = null
  failed to match
```

. 从代码可以读出, 这个算法不断推进活跃集, 并为每一个状态维护一个槽格局. 当传播时, 标记目标节点为活跃, 并把目标节点的槽格局更新为当前槽格局执行动作后的格局. 关键点在于交汇时, 当节点可以被多条路径激活(多条路径在此交汇)时, 算法通过一个仲裁函数 `arbitration` 选取哪一个, 并永远舍弃另一个.

我们先考虑算法的正确性. 算法是否会给出一个不可能的槽格局输出? 我们注意到每一个节点对应的槽格局一定是某条从开始节点到当前节点的路径, 如果槽格局被输出, 那么根据我们之前的叙述, 这个槽格局一定是可能的. 所以算法是正确和一致的.

我们再考虑仲裁函数的性质. 根据前文我们直到, 即使仲裁函数永远选第一个或者第二个, 或者随机选择, 算法最后也会给出可能的槽格局. 那么当可能槽格局数为1时(原表达式无歧义), 给出的输出总能正确匹配捕获组; 对于有歧义的情况, 由于 TNFA 和原表达式高度异构, 我们难以通过仲裁函数使得 TNFA 输出完全兼容消除歧义标准(例如贪心最长). 但我们可以通过一系列启发式指标和算法来尽可能匹配最长.

## 🧪 测试

```sh
make test -j20
```

## 🔗 依赖

已验证的编译器最低版本:

## Clang++

版本 >= 12

`--std=c++20` 或更高 (如果支持).

如果展开深度过深导致编译失败, 添加 `-fbracket-depth=[A BIG NUMBER] -ftemplate-depth=[A BIG NUMBER]`.

## g++

版本 >= 12

`--std=c++20` 或更高 (如果支持).

如果展开深度过深导致编译失败, 添加 `-ftemplate-depth=[A BIG NUMBER]`.

**注意, 编译复杂模式时, g++ 编译速度显著地比 clang++ 慢.**

## 😭 已知问题

❗ 现有实现为了可接受的编译时间, 不使用语义等价, 而使用语法等价来合并状态, 代价是不支持写存在多种可能解释的闭包, 例如 `(ab|ababab|c)*`, `(aa|aaa)*`, `(a*|aa)*` 或 `(a*aa)*`, 而应等价地写作 `(ab|c)*`, `|aaa*`, `a*` 或 `(aa)*`, 否则会导致编译失败. 或许未来会添加 Exact 模式来使得这种表达式的编译变得可能, 但可以预见编译时间将变得不可接受.

🩹 以上问题涉及的表达式几乎不会在正常使用中出现, 并且由于模式字符串是静态的, 所以也不会被外部利用攻击. 暂时不考虑修复.
