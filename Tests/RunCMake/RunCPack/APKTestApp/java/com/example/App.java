package com.example;

public class App {
    public static native int answer();

    public static int libraryName() {
        return com.example.lib.R.string.lib_name;
    }
}
