# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dewe/ViZDoom

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dewe/ViZDoom/build

# Include any dependencies generated for this target.
include src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/depend.make

# Include the progress variables for this target.
include src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/progress.make

# Include the compile flags for this target's objects.
include src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/flags.make

src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.o: src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/flags.make
src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.o: ../src/vizdoom/gdtoa/arithchk.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dewe/ViZDoom/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.o"
	cd /home/dewe/ViZDoom/build/src/vizdoom/gdtoa && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/arithchk.dir/arithchk.c.o   -c /home/dewe/ViZDoom/src/vizdoom/gdtoa/arithchk.c

src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/arithchk.dir/arithchk.c.i"
	cd /home/dewe/ViZDoom/build/src/vizdoom/gdtoa && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dewe/ViZDoom/src/vizdoom/gdtoa/arithchk.c > CMakeFiles/arithchk.dir/arithchk.c.i

src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/arithchk.dir/arithchk.c.s"
	cd /home/dewe/ViZDoom/build/src/vizdoom/gdtoa && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dewe/ViZDoom/src/vizdoom/gdtoa/arithchk.c -o CMakeFiles/arithchk.dir/arithchk.c.s

# Object files for target arithchk
arithchk_OBJECTS = \
"CMakeFiles/arithchk.dir/arithchk.c.o"

# External object files for target arithchk
arithchk_EXTERNAL_OBJECTS =

bin/arithchk: src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/arithchk.c.o
bin/arithchk: src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/build.make
bin/arithchk: src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dewe/ViZDoom/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../../../bin/arithchk"
	cd /home/dewe/ViZDoom/build/src/vizdoom/gdtoa && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/arithchk.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/build: bin/arithchk

.PHONY : src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/build

src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/clean:
	cd /home/dewe/ViZDoom/build/src/vizdoom/gdtoa && $(CMAKE_COMMAND) -P CMakeFiles/arithchk.dir/cmake_clean.cmake
.PHONY : src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/clean

src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/depend:
	cd /home/dewe/ViZDoom/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dewe/ViZDoom /home/dewe/ViZDoom/src/vizdoom/gdtoa /home/dewe/ViZDoom/build /home/dewe/ViZDoom/build/src/vizdoom/gdtoa /home/dewe/ViZDoom/build/src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/vizdoom/gdtoa/CMakeFiles/arithchk.dir/depend

