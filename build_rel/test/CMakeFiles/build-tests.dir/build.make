# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jyh/bustub-private

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jyh/bustub-private/build_rel

# Utility rule file for build-tests.

# Include any custom commands dependencies for this target.
include test/CMakeFiles/build-tests.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/build-tests.dir/progress.make

test/CMakeFiles/build-tests:
	cd /home/jyh/bustub-private/build_rel/test && /usr/bin/ctest --show-only

build-tests: test/CMakeFiles/build-tests
build-tests: test/CMakeFiles/build-tests.dir/build.make
.PHONY : build-tests

# Rule to build all files generated by this target.
test/CMakeFiles/build-tests.dir/build: build-tests
.PHONY : test/CMakeFiles/build-tests.dir/build

test/CMakeFiles/build-tests.dir/clean:
	cd /home/jyh/bustub-private/build_rel/test && $(CMAKE_COMMAND) -P CMakeFiles/build-tests.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/build-tests.dir/clean

test/CMakeFiles/build-tests.dir/depend:
	cd /home/jyh/bustub-private/build_rel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jyh/bustub-private /home/jyh/bustub-private/test /home/jyh/bustub-private/build_rel /home/jyh/bustub-private/build_rel/test /home/jyh/bustub-private/build_rel/test/CMakeFiles/build-tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/build-tests.dir/depend

