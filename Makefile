.PHONY: all
# all: elf elfgames/checkers elfgames/russian_checkers
all: elf elfgames/russian_checkers

.PHONY: clean
clean:
	rm -rf build/

.PHONY: test
test: test_cpp

.PHONY: test_cpp
test_cpp: test_cpp_elf

build/Makefile: CMakeLists.txt */CMakeLists.txt
	mkdir -p build
	(cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..)

.PHONY: re
re: clean all

.PHONY: elf
elf: build/Makefile
	(cd build && cmake --build elf -- -j)


.PHONY: test_cpp_elf
test_cpp_elf:
	(cd build/elf && GTEST_COLOR=1 ctest --output-on-failure)


# Games
.PHONY: elfgames/checkers
elfgames/checkers: build/Makefile
	(cd build && cmake --build elfgames/checkers -- -j)

.PHONY: elfgames/russian_checkers
elfgames/russian_checkers: build/Makefile
	(cd build && cmake --build elfgames/russian_checkers -- -j)