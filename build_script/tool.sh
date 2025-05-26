function SafeMakeDir() {
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
    fi
}

function SafeRmAndMkdir() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir -p "$1"
}
