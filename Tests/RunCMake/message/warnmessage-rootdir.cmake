cmake_minimum_required(VERSION 3.15)

# Generating the backtrace for this warning message used to trigger a
# spurious assertion when the current directory is the root directory
message(WARNING "We expect to see this warning message")
