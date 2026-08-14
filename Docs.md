# Docs

Everything from NES docs, current doubts and what's left to do

## Current State & Doubts

### State

- [X] Make a special carry flag cases depending on the arithemetic functions. Also stoped doing 2 complements because it was more dificult than helpful
- [ ] Need to review all instructions to check if they alter the right flags from the Processor Status (PS)
- [ ] Need to make all the illegal opcodes

### Doubts

- The 2 complement is done in compilation or is the cpu that does?

R: No the 2 complement is done in the cpu. The compiler understands -5 as 5 and a subtract instruction

## NES Docs

6502 CPU Instruction Set [link](https://www.pagetable.com/c64ref/6502/?tab=2)
