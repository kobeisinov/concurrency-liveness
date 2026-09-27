# 01 · Deadlock

**Deadlock:** threads wait for each other in a circle, forever. Nobody runs.

## The story

To sign a letter you need **both** the pen and the paper. Each item is a `std::mutex`.

- Alice takes the **pen** first, then the **paper**.
- Bob takes the **paper** first, then the **pen**.

## What happens, step by step ([problem.cpp](problem.cpp))

| time | Alice                        | Bob                          |
|------|------------------------------|------------------------------|
| 1    | locks the **pen** ✅          | locks the **paper** ✅        |
| 2    | wants the paper → Bob has it | wants the pen → Alice has it |
| 3    | waits…                       | waits…                       |
| ∞    | still waiting                | still waiting                |

Alice waits for Bob. Bob waits for Alice. That is a circle, so nobody can move.
The threads are not using the CPU. They are asleep and will never wake up.

```
$ ./01-deadlock/problem
Alice: took the pen, waiting for the paper...
Bob  : took the paper, waiting for the pen...

Nobody finished after 2 seconds -> DEADLOCK.
```

> `main()` gives up after 2 seconds, so the demo doesn't hang your terminal.
> A real program would just freeze.

## The fix ([solution.cpp](solution.cpp))

**Everybody locks in the same order.** One line changed:

```diff
- std::thread(sign, "Bob", std::ref(paper), std::ref(pen)).detach();
+ std::thread(sign, "Bob", std::ref(pen),   std::ref(paper)).detach();
```

Now both people want the pen first. Whoever gets it can also get the paper,
because the other person is still waiting for the pen and holds nothing.
With nothing held while waiting, there is no circle, so there is no deadlock.

```
$ ./01-deadlock/solution
Alice: took the pen, waiting for the paper...
Alice: signed!
Bob  : took the pen, waiting for the paper...
Bob  : signed!

Both signed. No deadlock.
```

## Remember

A deadlock needs all four of these at once. Break any one and it can't happen:

1. **Only one at a time.** A mutex has a single owner.
2. **Hold and wait.** A thread holds one lock while it waits for another.
3. **No taking away.** Nobody can force a thread to give up its lock.
4. **Circular wait.** A waits for B and B waits for A. ← *our fix breaks this one*

Tip: in real C++ code, `std::scoped_lock lock(pen.m, paper.m);` locks several
mutexes at once without deadlock, whatever order you list them in.
