#include "../include/id3.h"
#include <windows.h>
#include <iostream>
#include <memory>
#include <cstring>
#include <string>

using namespace std;

static const DWORD TAG_SIZE = 128;

#pragma pack(push, 1)
struct Id3v1Tag {
    char magic[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    char genre;
};
#pragma pack(pop)

static_assert(sizeof(Id3v1Tag) == 128, "Id3v1Tag must be exactly 128 bytes");

struct VirtualFreeDeleter {
    void operator()(BYTE* p) const {
        if (p) { VirtualFree(p, 0, MEM_RELEASE); }
    }
};

using VirtualBuffer = unique_ptr<BYTE[], VirtualFreeDeleter>;

static VirtualBuffer allocBuffer(SIZE_T size) {
    BYTE* ptr = static_cast<BYTE*>(
        VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    return VirtualBuffer(ptr);
}

static string trimField(const char* buf, int len) {
    string s(buf, static_cast<size_t>(len));
    auto end = s.find_last_not_of('\0');
    if (end == string::npos) { return ""; }
    s.erase(end + 1);
    end = s.find_last_not_of(' ');
    if (end == string::npos) { return ""; }
    s.erase(end + 1);
    return s;
}

static void fillField(char* dst, int len, const string& src) {
    memset(dst, 0, static_cast<size_t>(len));
    int copyLen = static_cast<int>(src.size()) < len
                      ? static_cast<int>(src.size())
                      : len;
    memcpy(dst, src.c_str(), static_cast<size_t>(copyLen));
}

bool readTag(const string& filepath) {
    HANDLE hFile = CreateFileA(
        filepath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "CreateFile failed (error " << GetLastError() << "): " << filepath << "\n";
        return false;
    }

    LARGE_INTEGER fileSize{};
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart < TAG_SIZE) {
        cout << "File is too small to contain an ID3v1 tag: " << filepath << "\n";
        CloseHandle(hFile);
        return false;
    }

    LARGE_INTEGER offset{};
    offset.QuadPart = -static_cast<LONGLONG>(TAG_SIZE);
    if (!SetFilePointerEx(hFile, offset, nullptr, FILE_END)) {
        cout << "SetFilePointerEx failed (error " << GetLastError() << ")\n";
        CloseHandle(hFile);
        return false;
    }

    VirtualBuffer buf = allocBuffer(TAG_SIZE);
    if (!buf) {
        cout << "VirtualAlloc failed\n";
        CloseHandle(hFile);
        return false;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buf.get(), TAG_SIZE, &bytesRead, nullptr) || bytesRead != TAG_SIZE) {
        cout << "ReadFile failed (error " << GetLastError() << ")\n";
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);

    if (buf[0] != 'T' || buf[1] != 'A' || buf[2] != 'G') {
        cout << "No ID3v1 tag found in: " << filepath << "\n";
        return false;
    }

    const Id3v1Tag* tag = reinterpret_cast<const Id3v1Tag*>(buf.get());
    cout << "Title:  " << trimField(tag->title,  30) << "\n"
         << "Artist: " << trimField(tag->artist, 30) << "\n"
         << "Album:  " << trimField(tag->album,  30) << "\n"
         << "Year:   " << trimField(tag->year,    4) << "\n";
    return true;
}

bool writeTag(const string& filepath) {
    HANDLE hFile = CreateFileA(
        filepath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "CreateFile failed (error " << GetLastError() << "): " << filepath << "\n";
        return false;
    }

    LARGE_INTEGER fileSize{};
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart < static_cast<LONGLONG>(TAG_SIZE)) {
        cout << "File is too small to contain an ID3v1 tag: " << filepath << "\n";
        CloseHandle(hFile);
        return false;
    }

    VirtualBuffer buf = allocBuffer(TAG_SIZE);
    if (!buf) {
        cout << "VirtualAlloc failed\n";
        CloseHandle(hFile);
        return false;
    }

    LARGE_INTEGER offset{};
    offset.QuadPart = -static_cast<LONGLONG>(TAG_SIZE);
    SetFilePointerEx(hFile, offset, nullptr, FILE_END);
    DWORD bytesRead = 0;
    ReadFile(hFile, buf.get(), TAG_SIZE, &bytesRead, nullptr);

    Id3v1Tag* tag = reinterpret_cast<Id3v1Tag*>(buf.get());
    bool hasTag = (bytesRead == TAG_SIZE
                   && buf[0] == 'T' && buf[1] == 'A' && buf[2] == 'G');

    if (!hasTag) {
        memset(tag, 0, TAG_SIZE);
        tag->magic[0] = 'T'; tag->magic[1] = 'A'; tag->magic[2] = 'G';
    }

    auto prompt = [&](const char* label, char* field, int fieldLen) {
        cout << label << " [" << trimField(field, fieldLen) << "]: ";
        string input;
        getline(cin, input);
        if (!input.empty()) {
            fillField(field, fieldLen, input);
        }
    };

    prompt("Title",  tag->title,  30);
    prompt("Artist", tag->artist, 30);
    prompt("Album",  tag->album,  30);
    prompt("Year",   tag->year,    4);

    LARGE_INTEGER writePos{};
    writePos.QuadPart = hasTag
        ? fileSize.QuadPart - static_cast<LONGLONG>(TAG_SIZE)
        : fileSize.QuadPart;

    if (!SetFilePointerEx(hFile, writePos, nullptr, FILE_BEGIN)) {
        cout << "SetFilePointerEx failed (error " << GetLastError() << ")\n";
        CloseHandle(hFile);
        return false;
    }

    DWORD bytesWritten = 0;
    if (!WriteFile(hFile, buf.get(), TAG_SIZE, &bytesWritten, nullptr) || bytesWritten != TAG_SIZE) {
        cout << "WriteFile failed (error " << GetLastError() << ")\n";
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    cout << "Tag written successfully.\n";
    return true;
}
