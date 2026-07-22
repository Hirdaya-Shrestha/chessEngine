## how to run:

**perft testing / benchmarking**: `clang++ -std=c++20 -O3 main.cpp -o main \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lraylib \
-framework IOKit \
-framework Cocoa \
-framework OpenGL`

i need the homebrew flags or else it wont detect raylib

**debug**: `clang++ -std=c++20 -g main.cpp -o main \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lraylib \
-framework IOKit \
-framework Cocoa \
-framework OpenGL`

**release**: `clang++ -std=c++20 -O3 -DNDEBUG main.cpp -o main \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lraylib \
-framework IOKit \
-framework Cocoa \
-framework OpenGL`