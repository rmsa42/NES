# Development

Here's a complete overview of everything i'm doing or have doubt. Whenever something gets done i will add an explanation about it

## Development Log

**15/08/2026**
- [X] Make special carry flag cases depending on the arithmetic functions.
> R: Also stoped doing 2 complements because it was more dificult than helpful

**18/08/2026**
- [X] Need to review all instructions to check if they alter the right flags from the Processor Status (PS)
> R: Checked every instruction, all of them check the right flags

- [ ] Make tests for all arithmetic instructions. Also need to find a structure for the tests (encapsulation of the cpu or total public)
- [ ] Need to review the PS flags functions, mostly the carry flag function
- [ ] Need to review the overflow flag in the SBC instruction
- [ ] Need to make all the illegal opcodes
- [ ] Need to rearrange the test cmake file whenever i structure the project into one or multiple libraries 

## Doubts

**13/08/2026**
The 2 complement is done in compilation or is the cpu that does?
> R: No the 2 complement is done in the cpu. The compiler understands -5 as 5 and a subtract instruction

Does the ROL instructions add the carry first and then shift or the other way around? Also does it set bit0 or adds the carry?
