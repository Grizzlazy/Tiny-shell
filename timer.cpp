#include <iostream>
#include <Windows.h>

DWORD WINAPI TimerThread(LPVOID lpParam) {
    int count = 1000;

    while (count > 0) {
        std::cout << count << std::endl;
        Sleep(1000); // �?i 1 gi�y
        count--;
    }

    return 0;
}

int main() {
    HANDLE hThread;
    DWORD threadId;

    // T?o ti?n tr�nh b? d?m th?i gian
    hThread = CreateThread(NULL, 0, TimerThread, NULL, 0, &threadId);

    // Ki?m tra xem vi?c t?o ti?n tr�nh th�nh c�ng hay kh�ng
    if (hThread == NULL) {
        std::cerr << "Failed to create timer thread." << std::endl;
        return 1;
    }

    // Ch? ti?n tr�nh ho�n th�nh
    WaitForSingleObject(hThread, INFINITE);

    // ��ng handle c?a ti?n tr�nh
    CloseHandle(hThread);

    return 0;
}

