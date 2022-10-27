find . -type d \( -path './build*' -or -path ./.github -or -path ./cmake \) -prune -o -name '*.cpp' -or -name '*.hpp' | xargs clang-format -style=file -i
