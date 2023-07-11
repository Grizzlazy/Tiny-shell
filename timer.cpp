#include <iostream>
#include <Windows.h>

DWORD WINAPI TimerThread(LPVOID lpParam) {
    int count = 1000;

    while (count > 0) {
        std::cout << count << std::endl;
        Sleep(1000); // Ð?i 1 giây
        count--;
    }

    return 0;
}

int main() {
    HANDLE hThread;
    DWORD threadId;

    // T?o ti?n trình b? d?m th?i gian
    hThread = CreateThread(NULL, 0, TimerThread, NULL, 0, &threadId);

    // Ki?m tra xem vi?c t?o ti?n trình thành công hay không
    if (hThread == NULL) {
        std::cerr << "Failed to create timer thread." << std::endl;
        return 1;
    }

    // Ch? ti?n trình hoàn thành
    WaitForSingleObject(hThread, INFINITE);

    // Ðóng handle c?a ti?n trình
    CloseHandle(hThread);

    return 0;
}

