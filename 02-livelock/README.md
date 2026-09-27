# 02 · Livelock

**Livelock:** threads keep running and reacting to each other, but no real work gets done.

Think of two people in a hallway who both step left, then both step right,
then both step left again. They are moving all the time, but nobody gets past.

## The story

Same as the deadlock: to sign you need **both** the pen and the paper.
This time both people are *polite* and never keep waiting while holding something:

> "If I can't get the second item, I put my first item back and try again."

This rule avoids the deadlock, but it creates a new problem.

## What happens, step by step ([problem.cpp](problem.cpp))

| time | Alice                                | Bob                                  |
|------|--------------------------------------|--------------------------------------|
| 1    | takes the **pen**                    | takes the **paper**                  |
| 2    | `try_lock` paper → busy ❌            | `try_lock` pen → busy ❌              |
| 3    | puts the pen back                    | puts the paper back                  |
| 4    | waits 50 ms                          | waits 50 ms                          |
| 5    | takes the **pen** again…             | takes the **paper** again…           |

Both wait **exactly the same time**, so they stay in step and repeat the same moves
forever. (`try_lock` never waits. It either gets the lock or returns `false` right away.)

Compared with deadlock: in a deadlock the threads **sleep** and do nothing.
In a livelock the threads are **awake and busy** but get nothing done.

```
$ ./02-livelock/problem
Alice: took the pen
Bob  : took the paper
Bob  : the pen is busy -> put the paper back, try again
Alice: the paper is busy -> put the pen back, try again
Bob  : took the paper
Alice: took the pen
...                       (same thing 5 times)
Alice: gave up after 5 tries. Busy the whole time, signed nothing -> LIVELOCK.
Bob  : gave up after 5 tries. Busy the whole time, signed nothing -> LIVELOCK.
```

> The demo stops after 5 tries so it ends. Without the limit, it would loop forever.

## The fix ([solution.cpp](solution.cpp))

**After a failed try, wait a random time.** One wait changed:

```diff
- std::this_thread::sleep_for(50ms);
+ std::this_thread::sleep_for(std::chrono::milliseconds(random_ms(rng)));  // 0..200 ms
```

Now Alice and Bob wait different amounts of time, so they fall out of step.
Soon one of them takes both items while the other one is still waiting.
This is called **random backoff**. Wi-Fi and Ethernet use the same trick when two devices
try to send at the same moment.

```
$ ./02-livelock/solution
Alice: took the pen
Bob  : took the paper
Alice: the paper is busy -> put the pen back, try again
Bob  : the pen is busy -> put the paper back, try again
...
Alice: took the pen
Alice: took the paper too -> signed! (attempt 4)
Bob  : took the paper
Bob  : took the pen too -> signed! (attempt 4)
```

## Remember

- A livelock looks like progress (CPU busy, lots of output), but no useful work gets done.
- The usual cause is threads that are **too symmetric**: same rule, same timing.
- To fix it, break the symmetry with random waits or a fixed order (like the deadlock fix).
