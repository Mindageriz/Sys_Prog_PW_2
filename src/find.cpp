#include "../include/find.h"
#include <windows.h>
#include <iostream>

using namespace std;

bool findMp3s(const string& dir) {
    string pattern = dir;
    if (!pattern.empty() && pattern.back() != '\\' && pattern.back() != '/') {
        pattern += '\\';
    }
    pattern += "*.mp3";

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            cout << "No MP3 files found in: " << dir << "\n";
            return true;
        }
        if (err == ERROR_PATH_NOT_FOUND || err == ERROR_INVALID_NAME) {
            cout << "Directory not found: " << dir << "\n";
            return false;
        }
        cout << "FindFirstFile failed (error " << err << ")\n";
        return false;
    }

    int count = 0;
    do {
        cout << "  " << fd.cFileName << "\n";
        ++count;
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    cout << count << " MP3 file(s) found.\n";
    return true;
}
