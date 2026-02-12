alias b := build
alias c := clean
alias d := debug
alias fc := fclean
alias r := run

binary_name := "farfield"

# Shows available receipes
default:
    @just --list

_config:
    cmake -B .build -S .

# Configure and build the project
build: _config
    cmake --build .build
    cp .build/{{ binary_name }} {{ binary_name }}

# Build and run the project
run: build
    @echo "========="
    @./{{ binary_name }}

# Remove build directory
clean:
    rm -rf .build

# Remove build directory && binary file
fclean: clean
    rm -f {{ binary_name }}

# Rebuild the tool each time a change/file add is detected
[linux]
debug:
    while sleep 1; do find src resources -type f | entr -cd bash -c 'just run'; done

# Rebuild the tool each time a change/file add is detected
[windows]
debug:
    watchexec -r -w src -w resources just run

# Run unit tests for the project
# test: build
