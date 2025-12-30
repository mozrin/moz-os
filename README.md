# moz-os

**moz-os** is a custom operating system kernel developed as a pioneering testbed for advanced **Autonomous AI Agentic Coding**.

## Development Methodology

This project demonstrates a novel software development workflow where architecture is defined by humans, but execution is driven by autonomous AI.

* **The Architect (User)**: Defines the high-level system architecture, strategic goals, and technical requirements.
* **The Engineer (AI Agent)**: Operates autonomously to implement the vision. The agent breaks down requirements into granular phases, writes implementation plans, creates code, debugs complex low-level issues (bootloaders, memory management), and verifies functionality—all with minimal human oversight.

The development follows a rigorous, phased approach documented in the `docs/` directory, moving from a blank slate to a functional 64-bit kernel capable of specific tasks.

## Technical Capabilities

Current implemented features include:

* **Custom Bootloader**: Two-stage bootloader (MBR + Stage 2) transitioning from Real Mode to 64-bit Long Mode.
* **Kernel Core**: Minimal 64-bit C kernel logic.
* **Drivers**: UART Serial driver for communication and debugging.
* **Networking Skeleton**: Basic structure for TCP/IP stack integration.
* **Stratum Application**: A functional mining loop that ingests Stratum jobs via UART, computes SHA-256 hashes, and submits valid shares.

## Project Structure

* `boot/` - Assembly source code for the bootloader chain.
* `kernel/` - C source code for the kernel, drivers, and application logic.
* `docs/` - Autonomous phase tracking documents (`phase_XXXX.txt`).
* `tests/` - Verification scripts and input/output vectors for QEMU testing.
