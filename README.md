# WinAPI Developer Calculator

This is a native Windows desktop application engineered in C++ using the Win32 API. Built as a specialized tool for developers, it provides hardware-accurate arithmetic across diverse C++ data types, serving as a practical environment for observing low-level computational behaviors such as integer overflow, sign-extension, and floating-point precision constraints.

> Project Origin: This application was developed as a comprehensive academic exercise in low-level systems programming. It successfully implements all advanced requirements of the curriculum, serving as a complete and highly optimized Win32 executable.

### Technical Implementation

Low-Level Window Management: Developed without high-level frameworks (like MFC or .NET), utilizing direct Win32 API calls for window procedure management, message looping, and resource handling.

Architecture-Specific Arithmetic: Implemented custom logic to handle 11 distinct primitive types ($int8$ through $double$), ensuring that calculations mirror the exact behavior of the underlying hardware architecture.

Resource-Efficient GUI: Utilized native GDI and resource scripts to create a zero-dependency executable with a minimal memory footprint.

Dynamic Configuration: Integrated a custom parser for config.ini files to allow runtime UI and environmental adjustments without source modification.

### Key Functionalities

Accurate Bit-Level Representation: Allows users to switch between signed and unsigned types to see how bits are interpreted by the CPU during arithmetic operations.

Precision Monitoring: Ideal for testing edge cases in financial or scientific applications where standard "consumer" calculators mask precision loss.

Theme & UI Customization: Supports external configuration for visual parameters via a standard initialization file.

Usage & Build Instructions

### Running the Application

Run WinApiCalc.exe

### Building from Source

Requirements: Visual Studio 2022 with the "Desktop development with C++" workload.

Open PiGE_LAB3.sln.

Set configuration to Release and your target architecture (x64 or x86).

Build the solution (Ctrl+Shift+B).

Deploy the compiled binary along with the provided config.ini.
