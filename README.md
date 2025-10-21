[![en](https://img.shields.io/badge/lang-en-green)](README.md)
[![zh-cn](https://img.shields.io/badge/lang-%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87-green)](README.zh-cn.md)

# ONRE

A header-only regex Engine with zero-cost abstraction and strictly O(n) time complexity.

---

## ✨ Core features

* ✔️ Strict $O(|s|)$ matching complexity, where $|s|$ denotes the length of the subject string.
* ✔️ Compile-time construction of automata for constant string regular expressions, leaving no runtime overhead.
* ✔️ Single-header file; just `include` to use.
* ✔️ **Supports capture groups** and replacement based on capture groups.
* ✔️ Uses Brzozowski derivatives with type-level functional metaprogramming to ensure fast compilation and almost always produce minimal automata, and **it’s cool**!
* ✔️ Supports all visible ASCII characters as the alphabet.
* ✔️ Supports standard regular expressions (concatenation, alternation `|`, Kleene star `*`, parentheses `()`); supports `+` and `?`; supports wildcard `.`; supports character classes (like `[^a-z012]`); supports escapes (`\n`, `\t`, `\d`, `\s`, `\w`, `\x[HEX]`, `\[`, `\]`, `\*`, ...); supports quantifiers (`{n}`, `{n,}`, `{,m}`, `{n,m}`).
* ❌️ Does not support zero-width assertions.
* ❌️ Does not support backreferences.
* ❌️ Capture disambiguation rules do not conform to POSIX or Perl standards.

## ⏱️ Performance showcase

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

For very long strings, catastrophic backtracking cases, very long patterns, and very complex patterns the engine still reliably achieves O(n) performance; the same test cases on backtracking engines almost always blow up (for example the famous [Cloudflare incident](https://www.reddit.com/r/sysadmin/comments/c8eymj/cloudflare_outage_caused_by_deploying_bad_regular/)).

Compiling all of these (many more than shown; see `{[...]}`) complex patterns is controllable and acceptable:

```sh
> make clean && time make -j30
real    0m13.845s
user    1m19.445s
sys     0m3.850s
```

## 🤔 Usage

```cpp
#include "onre.hpp"
#include <string>

void f() {
  bool result = onre::Match<"((ab)*|c*|b)(@\\.)?">::eval("abab");
  std::string replaced = onre::Replace<"ab(.*)ab">::eval("$0, $1, $$", "ababab");
  // result = true; replaced = "ababab, ab, $"
}
```

`onre::Replace` uses `$N` to refer to the N-th capture group (ordered by the position of the left parenthesis, starting from 1). `$0` denotes the whole string. Use `$$` to represent a literal `$`.

If `onre::Replace` cannot match, the `eval()` result is undefined; if the replacement rule is invalid, `eval()` returns an empty string. Therefore, for cases where a match might fail, `onre::Match` should be invoked first to check.

Instantiating `onre::Match` or `onre::Replace` anywhere in the code will trigger compile-time expansion and increase compile time even if the instance can never be executed at runtime. The same pattern instantiated multiple times within one translation unit is instantiated only once; different translation units will each instantiate it separately, so moving complex patterns into a single translation unit can greatly reduce compile time.

```cpp
#include "onre.hpp"

bool is_valid_email(std::string email) {
  // only compiled once
  return onre::Match<"[-a-zA-Z0-9.]+@([-a-zA-Z0-9]+.)+[-a-zA-Z0-9]+">::eval(email);
}
```

For programs that use very complex expressions, the compile options should include large bracket/template depth limits `-fbracket-depth=[A BIG NUMBER] -ftemplate-depth=[A BIG NUMBER]` to allow deeper compile-time recursion.

## ⚖️ Standard

Unlike POSIX (IEEE Std 1003.1-2001) leftmost-longest semantics, ONRE follows a rightmost-longest disambiguation rule. Concretely, captures inside closures match the last possible substring that completes the match. For example, when matching `((a*)(a*)b)*` against `aaababaaaab`, the results are:

| Capture | POSIX | ONRE |
| - | - | - |
| `$1` (`((a*)(a*)b)`) | `aaab` | `aaaab` |
| `$2` (first `(a*)`) | `aaa` | `aaaa` |
| `$3` (second `(a*)`) | | |

Equivalently:

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

Other examples:

```cpp
onre::Replace<"((a*)b)*">::eval("$2", "aabb") => "" // aab-()-b
onre::Replace<"(a|ab)+b">::eval("$1", "abab") => "a" // ab-(a)-b
```

## 🤯 Theoretical Foundation

### Brzozowski Derivative

For a regular expression $R$ and a character $x$, we denote the Brzozowski derivative of $R$ with respect to $x$ as $\frac{\partial R}{\partial x}$ (hereafter abbreviated as the derivative). The language represented by this derivative is:

$$L(\frac{\partial R}{\partial x}) = \lbrace w \in \Sigma^*: xw \in L(R)\rbrace$$

—that is, after consuming $x$, it represents all strings $w$ that allow $R$ to complete a match. Its recursive definition is as follows:

$$\frac{\partial \emptyset}{\partial x} = \emptyset$$

$$\frac{\partial \epsilon}{\partial x} = \emptyset$$

$$\frac{\partial y}{\partial x} =\begin{cases}
\epsilon,\qquad y = x \\
\emptyset,\qquad \text{otherwise}
\end{cases} $$

$$\frac{\partial (R|S)}{\partial x} = \frac{\partial R}{\partial x} | \frac{\partial S}{\partial x}$$

$$\frac{\partial (RS)}{\partial x} = \frac{\partial R}{\partial x} S | \delta(R) \frac{\partial S}{\partial x}$$

$$\frac{\partial (R^{\ast})}{\partial x} = \frac{\partial R}{\partial x} R^{\ast}$$

where $\delta(\cdot)$ is the nullability function, defined as:

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

Based on this, we can construct a deterministic finite automaton (DFA) for expression $R$, denoted as $M = (Q, \Sigma, \delta, q_0, Q_{\text{accept}})$, where

$$Q \subset \text{RE}$$

$$\delta(R, x) = \frac{\partial R}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace R \in RE: \epsilon \in L(R)\rbrace$$

We recursively compute the derivatives of the regular expression until no new expressions are generated, completing the construction.

### Action Algebra

Consider $N$ slots $s_0, s_1, s_2, \ldots, s_{N-1}$, each capable of storing a natural number or -1 (indicating uninitialized). We denote setting slot $i$ to $x$ as $\text{set}(i, x)$.

If there are two actions, we define their concatenation $\alpha \cdot \beta$ as performing $\alpha$ first, followed by $\beta$. We also define $\omega$ as the empty action, meaning "do nothing", and $\alpha \cdot \omega = \omega \cdot \alpha = \alpha$. Such a structure is called an action algebra.

Formally, a slot configuration is an $N$-tuple $(s_0, s_1, \ldots, s_{N-1})$, where $s_i \in \mathbb N \cup {-1}$. The set of all slot configurations is $S_N = (\mathbb N \cup {-1})^N$.

If a set $A_N \subset S_N\,^{S_N}$ satisfies:

1. $\omega \in A, \text{where } \omega(S) = S$;
1. $\text{set}(i, x) \in A, \forall i \in \lbrace 0, 1, ..., N-1 \rbrace, x \in \mathbb N, \text{where } \text{set}(i, x)(s_0, s_1, ..., s_i, ..., s_{N-1}) = (s_0, s_1, ..., x, ..., s_{N-1})$;
1. $\alpha \cdot \beta \in A, \forall \alpha, \beta \in A, \text{where } \alpha \cdot \beta (T) = \beta(\alpha(T)) $,

then $A_N$ is called an action set.

### Extended Regular Expressions

If we allow a symbol `<i>` to appear in a regular expression, representing a zero-width (non-consuming) match that executes the action $\text{set}(i, p)$—where $p$ is the current consumed character count (i.e., the "cursor" position)—then we call such an expression an extended regular expression.

Formally, given a natural number $N$ and an alphabet $\Sigma$, if a set $E$ satisfies:

1. $\emptyset \in E$;
1. $\epsilon \in E$;
1. $ \langle i \rangle \in E, \forall i \in \lbrace 0, 1, 2, ..., N-1 \rbrace $;
1. $a \in E, \forall a \in \Sigma$;
1. $R|S \in E, \forall R, S \in E$;
1. $RS \in E, \forall R, S \in E$;
1. $R^\ast \in E, \forall R$,

then $E$ is called the extended regular expression set with $N$ slots over alphabet $\Sigma$.

Each extended regular expression defines a language, with $L(\langle i\rangle) = \lbrace \epsilon \rbrace$; all other rules follow the same semantics as standard regular expressions.

At the start of matching, we initialize the slot state as $(-1, -1, \ldots, -1)$. When matching, each subexpression executes its corresponding action upon completion. The resulting slot configuration after a full match is the output of the expression for that string. The output may not be unique (multiple paths may exist), and we do not attempt to disambiguate.

For example, the POSIX regular expression `a(a*)a`, which captures the substring excluding the first and last characters, can be viewed as the extended expression $\langle 0\rangle a^\ast\langle 1\rangle a$. Upon completion, it yields an output $(s_0, s_1)$, representing the start and end indices (left-open, right-closed) of the captured group.

### The $v$ Function

For an extended regular expression $R$, $v(R)$ gives the set of possible actions when $R$ accepts the empty string. If $R$ does not accept the empty string, $v(R)$ is empty. The recursive definition is:

$$ v(\emptyset) = \emptyset $$

$$ v(\epsilon) = \lbrace \omega \rbrace $$

$$ v(\langle i\rangle) = \lbrace \text{set}(i, p) \rbrace $$

$$ v(a) = \emptyset $$

$$ v(R|S) = v(R) \cup v(S) $$

$$ v(RS) = \lbrace \alpha \cdot \beta : \alpha \in v(R), \beta \in v(S) \rbrace$$

$$ v(R^\ast) = \lbrace \omega \rbrace $$

### Extended Brzozowski Derivative

For an extended regular expression $R$ and a character $x$, the **extended Brzozowski derivative** $\frac{\partial R}{\partial x}$ is defined as the set of all possible pairs ((S, \alpha)), where each ((S, \alpha)) means that $R$ can first execute action $\alpha$, then consume $x$, and finally continue matching using $S$. The recursive definition is:

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

### TNFA and Derivative Construction

Building upon the NFA, if we assign an *action* to every transition—executed before consuming the character and performing the state transition—we obtain a Tagged Nondeterministic Finite Automaton (TNFA).
The accepting states also have associated *accept actions*, executed when the automaton accepts a string.

Formally, a TNFA is a 7-tuple
$(Q, \Sigma, N, \delta, q_0, Q_\text{accept}, A_\text{accept})$,
where $Q$ is the set of states, $\Sigma$ the alphabet, $N$ the number of slots,
$\delta: Q \times \Sigma \to 2^{Q \times A_N}$ describes transitions and their actions,
$q_0$ is the initial state, $Q_\text{accept}$ is the accepting state set,
and $A_\text{accept}: Q_\text{accept} \to A_N$ gives the accepting actions.

We can construct a TNFA equivalent to an extended regular expression $R$ using the extended Brzozowski derivative, setting:

$$Q \subset \text{Extended RE}$$

$$N = \text{Number of Unique Slots in } R$$

$$\delta(q, x) = \frac{\partial q}{\partial x}$$

$$q_0 = R$$

$$Q_\text{accept} = \lbrace q : \epsilon \in L(q) \rbrace$$

$$A_\text{accept}(q) = v(q)$$

Note that $A_\text{accept}$ may contain ambiguities (multiple possible actions), which technically violates the TNFA definition, but we temporarily allow it. We will later discuss how to select one among them.

### Active-State TNFA Simulation for Capturing Groups

Backtracking simulation of a TNFA has exponential complexity—no better than a backtracking regex engine (and potentially slower). To constrain complexity, we use an *active-state NFA simulation method* and introduce a heuristic arbitration mechanism to resolve ambiguities.

Conceptually, given a TNFA and an accepted string, every path from the start to an accepting state corresponds to one possible slot configuration—the output of the extended regular expression for that string.

For instance, in `(a*)(a*)` (i.e., $\langle 0\rangle a^\ast\langle 1\rangle\langle 2\rangle a^\ast\langle 3\rangle$, different paths yield different outputs, but for each output, $\langle 0\rangle, \langle 1\rangle$ correctly mark the boundaries of the first `a*`, and $\langle 2\rangle, \langle 3\rangle$ those of the second, without overlap. This is intuitively evident, though giving a formal proof is difficult.

Now, consider the algorithm:
The TNFA has $S$ states labeled $q_0, q_1, \ldots, q_{S-1}$, with $q_0$ as the start state and $N$ slots.
We maintain `bool is_active[S]` and `number slots[S][N]`,
where `slots[i]` stores the slot configuration of state $i$.

Initially:

```text
is_active <- all false
is_active[0] <- true
slots[i][j] <- all -1
```

Then, for each consumed character `c`:

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

If no characters remain:

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

From the code, we see that the algorithm continually advances the active set and maintains a slot configuration for each state. During propagation, the destination state is marked active, and its slots are updated by executing the transition’s action.
At convergence points (when multiple paths activate the same state), an arbitration function decides which configuration to keep, permanently discarding others.

Consider the correctness first. Each state’s slot configuration corresponds to some valid path from the start to that state. Therefore, any output slot configuration must correspond to a valid path and is thus consistent with the original extended regular expression—ensuring correctness.

On the arbitration function, even if the arbitration function always picks the first, the second, or randomly, the final slot configuration will still represent a possible match.
Thus, when there is only one possible configuration (i.e., the expression is unambiguous), the output always correctly represents capture groups.
For ambiguous expressions, because the TNFA structure does not fully mirror the original regex semantics, it is difficult to design an arbitration rule perfectly matching disambiguation criteria (e.g., greedy longest).
However, we can approximate it through heuristic indicators and algorithms to favor the longest possible match.

## 🧪 Tests

```sh
make test -j20
```

## 🔗 Dependencies

Verified lowest compiler versions:

## Clang++

Version >= 12

`--std=c++20` or higher (if supported).

If recursion depth causes compile failures, add `-fbracket-depth=[A BIG NUMBER] -ftemplate-depth=[A BIG NUMBER]` to allow deeper compile-time recursion.

## g++

Version >= 12

`--std=c++20` or higher (if supported).

If recursion depth causes compile failures, add `-ftemplate-depth=[A BIG NUMBER]` to allow deeper compile-time recursion.

**Note that g++ compiles significantly slower than clang++ when compiling complex patterns.**

## 😭 Known issues

❗ To keep compilation time acceptable, the current implementation does not use semantic equivalence when merging states; it uses syntactic equivalence instead. As a result it does not support some expressions that have multiple interpretations inside closures, examples include `(ab|ababab|c)*`, `(aa|aaa)*`, `(a*|aa)*`, or `(a*aa)*`. Instead, equivalent forms like `(ab|c)*`, `|aaa*`, `a*`, or `(aa)*` are preferred; otherwise compilation may fail. An Exact mode might be added in future to relax this, but compilation time will predictably become unacceptable.

🩹 The expressions that cause the issue are rarely used in normal circumstances, and because patterns are static they are not exploitable for attack vectors. Fixing this is not currently planned.
