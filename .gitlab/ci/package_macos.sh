cd build
cpack -G TGZ
cpack -G DragNDrop

case "$CMAKE_CI_PACKAGE" in
    dev)
        ;;
    *)
        mkdir -p unsigned
        mv cmake-*-macos*-universal.* unsigned/
        ;;
esac
