#include <iostream>
#include <string>
#include "../include/find.h"
#include "../include/id3.h"

using namespace std;

static void usage(const char* prog) {
    cout << "Usage:\n"
         << "  " << prog << " --find  <directory>   List MP3 files in directory\n"
         << "  " << prog << " --read  <file.mp3>    Display ID3v1 tag metadata\n"
         << "  " << prog << " --write <file.mp3>    Edit ID3v1 tag metadata\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const string flag = argv[1];

    if (flag == "--find") {
        if (argc < 3) {
            cout << "--find requires a directory path\n";
            return 1;
        }
        return findMp3s(argv[2]) ? 0 : 1;
    }

    if (flag == "--read") {
        if (argc < 3) {
            cout << "--read requires a file path\n";
            return 1;
        }
        return readTag(argv[2]) ? 0 : 1;
    }

    if (flag == "--write") {
        if (argc < 3) {
            cout << "--write requires a file path\n";
            return 1;
        }
        return writeTag(argv[2]) ? 0 : 1;
    }

    cout << "Unknown flag: " << flag << "\n\n";
    usage(argv[0]);
    return 1;
}
