.PHONY: all
all: elf elfgames/go elfgames/checkers

.PHONY: clean
clean:
	rm -rf build/

.PHONY: re
re:	clean all

.PHONY: test
test: test_cpp

.PHONY: test_cpp
test_cpp: test_cpp_elf test_cpp_elfgames_go

build/Makefile: CMakeLists.txt */CMakeLists.txt
	mkdir -p build
	(cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..)

.PHONY: elf
elf: build/Makefile
	(cd build && cmake --build elf -- -j)

.PHONY: test_cpp_elf
test_cpp_elf:
	(cd build/elf && GTEST_COLOR=1 ctest --output-on-failure)


# Go game
.PHONY: test_cpp_elfgames_go
test_cpp_elfgames_go:
	(cd build/elfgames/go && GTEST_COLOR=1 ctest --output-on-failure)

.PHONY: elfgames/go
elfgames/go: build/Makefile
	(cd build && cmake --build elfgames/go -- -j)


# Checkers game
.PHONY: test_cpp_elfgames_checkers
test_cpp_elfgames_checkers:
	(cd build/elfgames/checkers && GTEST_COLOR=1 ctest --output-on-failure)

.PHONY: elfgames/checkers
elfgames/checkers: build/Makefile
	(cd build && cmake --build elfgames/checkers -- -j)
