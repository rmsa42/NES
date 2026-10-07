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

**28/08/2026**
- [X] Start with the APU (Audio Processing Unit) part
> R: After researching, i figured that the apu files will be more of a bridge between SDL and the memory values. I am still not sure about this so this could change in the future

**29/08/2026**
- [X] Migrated the development to Windows instead of WSL, because of SDL3 graphical parts
> R: This migration is explained in this [doc](Migration_to_windows.md)

**22/09/2026**
- [X] Pass the cpu global variable to the stack rather than malloc it
> R: There's no need to malloc the cpu global. Just put it on the stack

**22/09/2026**
- [X] Make a debug screen which can run an instruction at a time with debug info
> R: Made a super simple debug window just to catch cpu instruction bugs.
I fumbled a lot, so the code is super messy

**22/09/2026**
- [X] Organize Debug tools into their own file
> R: It's all in the **debug_tools** file

**22/09/2026**
- [X] Need to study and make NMI interrupts
> R: Studying it i found the NMI line is connected from the CPU to the PPU only, so i will make the PPU first and then the NMI.
This is mostly used to detected vertical blanking, so visual glitches don't happen and the CPU can safely write to VRAM

**28/09/2026**
- [X] Need to sort out the git stash thing

**01/10/2026**
- [X] Need to make a file to handle memory. Also remove memory varaibles from being global in other files than the *unit* they pertence
> R: Now the memory is handled via **bus**. This made possible to remove the memory global variable from other files, except the **rom.c**.
Now the memory-mapped registers are right

**01/10/2026**
- [X] Fix the **tests** CMake file. Bug: Include the SDL library to compile the files
> R: For now it will stay in the repo root. Whenever i refactor the code to take SDL out of nes units code (ppu, apu), i will put the tests make into their folder

**03/10/2026**
- [X] Implement the *bus* in the asl, lsr, rol and ror after making tests for the instructions
> R: Done

- [X] Fully refactor **debug_tools** with a new GUI library or just try to make a GUI with SDL3 :)
> R: This is not happening. I want to try and make something myself with future plans in mind

- [ ] Need to finish the **apu_write** function to integrate the apu with the bus
- [ ] Need to make the memory *mirroring*
- [ ] Need to switch the **nes_header** struct flags to a single byte variable
- [ ] Need to make the Sweep, envelope loop, const volume, volume and length counter load in apu for the pulses
- [ ] Need to finish the noise wave
- [ ] Find a way to set the lin_reload_flag in the triangule wave in apu
- [ ] Need to implement the decimal calculations from arithmetic instructions
- [ ] Review and possible implement what's left from the system vectors
- [ ] Need to make all the illegal opcodes
- [ ] Need to rearrange the test cmake file whenever i structure the project into one or multiple libraries 

## Doubts

**13/08/2026**
- [X] The 2 complement is done in compilation or is the cpu that does?
> R: No the 2 complement is done in the cpu. The compiler understands -5 as 5 and a subtract instruction

**22/08/2026**
- [X] Does the ROL instructions add the carry first and then shift or the other way around? Also does it set bit0 or adds the carry?
> R: The ROL and ROR instruction **sets** the bit with the old carry flag, so it doesn't add. And the **set** part is the last part of the instruction

- [ ] Does the timer low and timer high ($4002-$4003) update the APU timer every cycle or when the current timer reaches 0?
