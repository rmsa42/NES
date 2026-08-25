# Development

Here's a complete overview of everything i'm doing or have doubt. Whenever something gets done i will add an explanation about it

## Development Log

**15/08/2026**
- [X] Make special carry flag cases depending on the arithmetic functions.
> R: Also stoped doing 2 complements because it was more dificult than helpful

**18/08/2026**
- [X] Need to review all instructions to check if they alter the right flags from the Processor Status (PS)
> R: Checked every instruction, all of them check the right flags

**23/08/2026**
- [X] Make tests for all arithmetic instructions. Also need to find a structure for the tests (encapsulation of the cpu or total public)
> R: The most important instructions to test are the **ADC, SBC, CMP**, so i didn't make tests for the **CPX, CPY**. I opted to make the cpu total public so no getters/setters

**23/08/2026**
- [X] Need to review the overflow flag in the SBC instruction
> R: The implementation was wrong, i nedded to pass the number ones-complemented (without adding the carry) to my overflow check function

**25/08/2026**
- [X] Need to review the PS flags functions and macros, mostly the carry flag function
> R: I am not altering the PS flags functions for now, I will stay with what works. Just altered the macro names

- [ ] Need to make all the illegal opcodes
- [ ] Need to rearrange the test cmake file whenever i structure the project into one or multiple libraries 

## Doubts

**13/08/2026**
The 2 complement is done in compilation or is the cpu that does?
> R: No the 2 complement is done in the cpu. The compiler understands -5 as 5 and a subtract instruction

**22/08/2026**
Does the ROL instructions add the carry first and then shift or the other way around? Also does it set bit0 or adds the carry?
> R: The ROL and ROR instruction **sets** the bit with the old carry flag, so it doesn't add. And the **set** part is the last part of the instruction
