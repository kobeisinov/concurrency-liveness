# Concurrency liveness: deadlock, livelock, starvation

Three ways a multi-threaded program can fail to make progress. Each folder has
the **smallest program that shows the bug**, the **same program fixed**, and a
short README that walks through it step by step.

|                        | [01 · Deadlock](01-deadlock/)                 | [02 · Livelock](02-livelock/)                     | [03 · Starvation](03-starvation/)                       |
|------------------------|-----------------------------------------------|---------------------------------------------------|---------------------------------------------------------|
| **In one line**        | Threads wait for each other forever           | Threads keep reacting to each other, no progress  | Some threads almost never get a turn                    |
| **Everyday picture**   | Two people each hold what the other needs     | Two people in a hallway both step aside, again and again | Someone keeps cutting in line                    |
| **Are threads running?** | No, all asleep                              | Yes, busy all the time                            | Some are, the rest keep waiting                         |
| **Demo story**         | Alice: pen→paper, Bob: paper→pen              | Same, but "put it back and retry" in lockstep     | 3 workers share 1 printer (`std::mutex`)                |
| **Cause**              | Locks taken in a different order (circle)     | Same rule and same timing for everyone            | The lock is unfair; whoever just unlocked grabs it again |
| **Fix**                | Take locks in the **same order**              | Wait a **random** time before retrying            | Use a **fair lock** (ticket lock, first come first served) |
| **Lines changed**      | 1                                             | 1 (plus the random generator)                     | 1 (plus the `TicketLock` class)                         |

**Liveness** means "something good eventually happens". These are the three
classic ways it fails. (Race conditions are a different family. They break
*correctness*, not progress.)

## Build & run

Needs a C++17 compiler (g++ or clang++). Works on Linux and macOS.

```bash
make          # builds all 6 programs
make run      # runs every problem, then its solution
make clean    # removes the binaries
```

Or run one at a time:

```bash
./01-deadlock/problem     ./01-deadlock/solution
./02-livelock/problem     ./02-livelock/solution
./03-starvation/problem   ./03-starvation/solution
```

Every program finishes in about 2 seconds. The deadlock and livelock demos
stop themselves on purpose so they don't hang your terminal.

## How to read this repo

1. Open `problem.cpp`. The comment at the top tells the story.
2. Run it and watch the output.
3. Read the folder's `README.md` for the step-by-step timeline.
4. Diff it with the fix: `diff 01-deadlock/problem.cpp 01-deadlock/solution.cpp`.
   The change is always small.
