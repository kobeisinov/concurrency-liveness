# 03 · Starvation

**Starvation:** the program as a whole makes progress, but some threads almost never get a turn.

## The story

3 workers share **one printer** (a `std::mutex`). They all run the same loop:

```cpp
while (!stop) {
    printer.lock();
    turns[id]++;
    sleep_for(50ms);     // using the printer
    printer.unlock();    // ...and right away back to printer.lock()
}
```

The code is fair. The lock is not.

## What happens, step by step ([problem.cpp](problem.cpp))

1. Worker 0 locks the printer. Workers 1 and 2 go to **sleep**, waiting for the lock.
2. Worker 0 unlocks. The OS starts waking up a sleeping worker, and waking up takes time.
3. Worker 0 is **already running**. It loops and calls `lock()` again right away.
4. The lock is free, so worker 0 gets it again. The worker that was waking up finds it taken
   and goes back to sleep.
5. Repeat. One worker takes almost every turn.

`std::mutex` gives no promise of fairness. It does **not** serve threads in arrival order.

```
$ ./03-starvation/problem
Turns each worker got in 2 seconds (all 3 run the same code):
  worker 0: 37 turns
  worker 1:  1 turns
  worker 2:  1 turns
```

Workers 1 and 2 are **starving**. They are not stuck forever the way a deadlock is,
but in practice their turn never comes. (Numbers change each run, and on some
systems it is less extreme, but the imbalance is the point.)

## The fix ([solution.cpp](solution.cpp))

**Use a fair lock.** Only the type of `printer` changed:

```diff
- std::mutex printer;
+ TicketLock printer;
```

`TicketLock` works like the **"take a number"** machine at a bank:

```cpp
void lock() {
    int my_ticket = next_ticket++;                // take a number
    while (now_serving != my_ticket) yield();     // wait until it's called
}
void unlock() { now_serving++; }                  // call the next number
```

When worker 0 finishes and calls `lock()` again, it gets a **new** number at the
back of the line. It cannot skip ahead of workers who were already waiting.

```
$ ./03-starvation/solution
Turns each worker got in 2 seconds (all 3 run the same code):
  worker 0: 14 turns
  worker 1: 13 turns
  worker 2: 13 turns
```

## Remember

- Starvation comes from an **unfair rule** for who goes next: an unfair lock,
  strict priorities, or "readers always first".
- Fix it by making the rule fair. Serve in arrival order (FIFO / ticket lock), or let
  waiting threads gain priority the longer they wait (called **aging**).
- Fairness has a cost. Handing the lock to a sleeping thread is slower than letting a
  running thread take it again. That is why `std::mutex` is not fair by default.
