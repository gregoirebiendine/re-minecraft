alias b := build
alias c := clean
alias d := debug
alias fc := fclean
alias r := run

binary_name := "farfield"

# Shows available receipes
default:
    @just --list

[private]
config:
    cmake -B .build -S .

# Configure and build the project
[group: 'run']
build: config
    cmake --build .build
    cp .build/{{ binary_name }} {{ binary_name }}

# Build and run the project
[group: 'run']
run: build
    @echo "========="
    @./{{ binary_name }}

# Remove build directory
[group: 'clean']
clean:
    rm -rf .build

# Remove build directory && binary file
[group: 'clean']
fclean: clean
    rm -f {{ binary_name }}

# Rebuild the tool each time a change/file add is detected
[linux]
[group: 'dev']
debug:
    while sleep 1; do find src resources -type f | entr -cd bash -c 'just run'; done

# Rebuild the tool each time a change/file add is detected
[windows]
[group: 'dev']
debug:
    watchexec -r -w src -w resources just run

# Run unit tests for the project
# test: build
