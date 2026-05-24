find include -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
find src -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
find drivers -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
find examples -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
find perf -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
find tests -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
cmake-format -i CMakeLists.txt
