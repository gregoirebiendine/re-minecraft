alias b := build
alias c := clean
alias d := debug
alias fc := fclean
alias r := run

set windows-shell := ["powershell.exe", "-c"]

binary_name := if os() == "windows" { "farfield.exe" } else { "farfield" }

# Shows available receipes
default:
    @just --list

[private]
config:
    cmake -B .build -S .

# Configure and build the project
[group('run')]
build: config
    cmake --build .build

# Build and run the project
[group('run')]
[windows]
run: build
    @echo "========="; cd .build; ./Debug/{{ binary_name }}

[group('run')]
[linux]
run: build
    @echo "========="
    @./.build/{{ binary_name }}

# Remove build directory
[group('clean')]
clean:
    rm -rf .build

# Rebuild the tool each time a change/file add is detected
[group('dev')]
[linux]
debug:
    while sleep 1; do find src resources -type f | entr -cd bash -c 'just run'; done

# Rebuild the tool each time a change/file add is detected
[group('dev')]
[windows]
debug:
    watchexec -r -w src -w resources just run

# Run unit tests for the project
# test: build
