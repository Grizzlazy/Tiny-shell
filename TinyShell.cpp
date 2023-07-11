#include<iostream>
#include<windows.h>
#include<string>
#include<stdio.h>
#include<sstream>
#include<stdlib.h>
#include<cstring>
#include<ctime>
#include<direct.h>
#include <tlhelp32.h>
#include <iomanip>
#include <fstream>
#include<chrono>
#include<thread>
#include<conio.h>

using namespace std;


const char* timerPath = "C:\\Users\\Admin\\Desktop\\Tinyshell\\timer.exe";


void help() {
    cout <<setw(25)<<left<< "1. help"<<"Provide Help information for Windows commands" << endl;
    cout <<setw(25)<<left<< "2. time"<<"Display time" << endl;
    cout <<setw(25)<<left<< "3. date"<<"Display date" << endl;
    cout <<setw(25)<<left<< "4. path"<<"Display the current system path" << endl;
    cout <<setw(25)<<left<< "5. addpath"<<"Add a directory to the system path" << endl;
    cout <<setw(25)<<left<< "6. dir"<<"Display list of files in the current directory" << endl;
    cout <<setw(25)<<left<< "7. list"<<"List all running processes" << endl;
    cout <<setw(25)<<left<< "8. kill"<<"Terminate a process with specified PID" << endl;
    cout <<setw(25)<<left<< "9. stop"<<"Suspend a process with specified PID" << endl;
    cout <<setw(25)<<left<< "10. resume"<<"Resume execution of a suspended process with specified PID" << endl;
    cout <<setw(25)<<left<< "11. cd"<<"Change current directory to the specified path" << endl;
    cout <<setw(25)<<left<< "12. readbat"<<"Execute commands from a .bat file" << endl;
    cout <<setw(25)<<left<< "13. timer"<<"A simple timer" << endl;
    cout <<setw(25)<<left<< "14. exit"<<"Exit the program" << endl;
}
bool g_ExitRequested = false;
HANDLE g_ForegroundProcess = NULL;

void ListProcesses() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cout << "Failed to create process snapshot." << endl;
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32)) {
        cout << "Failed to retrieve process information." << endl;
        CloseHandle(hProcessSnap);
        return;
    }

    // Print process information
    cout << setw(50) << left << "Process Name"
         << setw(20) << left << "Process ID"
         << setw(20) << left << "Parent Process ID" << endl;
    cout << "-----------------------------------------------------------------------------------------------------"<<endl;

    do {
        cout << setw(50) << left << pe32.szExeFile
             << setw(20) << left << pe32.th32ProcessID
             << setw(20) << left << pe32.th32ParentProcessID << endl;
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void KillProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        cout << "Failed to open process." << endl;
        return;
    }

    if (TerminateProcess(hProcess, 0)) {
        cout << "Process terminated successfully." << endl;
    } else {
        cout << "Failed to terminate process." << endl;
    }

    CloseHandle(hProcess);
}

void SuspendProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess == NULL) {
        cout << "Failed to open process." << endl;
        return;
    }

    // Ð?u tiên, l?y handle c?a t?t c? các lu?ng (threads) trong ti?n trình
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        cout << "Failed to create thread snapshot." << endl;
        CloseHandle(hProcess);
        return;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // L?p qua t?ng lu?ng d? dình ch? (suspend)
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (hThread != NULL) {
                    // Ðình ch? lu?ng
                    SuspendThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);
    CloseHandle(hProcess);

    cout << "Process suspended successfully." << endl;
}

void ResumeProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (hProcess == NULL) {
        cout << "Failed to open process." << endl;
        return;
    }

    // Ð?u tiên, l?y handle c?a t?t c? các lu?ng (threads) trong ti?n trình
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        cout << "Failed to create thread snapshot." << endl;
        CloseHandle(hProcess);
        return;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // L?p qua t?ng lu?ng d? ti?p t?c (resume)
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (hThread != NULL) {
                    // Ti?p t?c lu?ng
                    ResumeThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);
    CloseHandle(hProcess);

    cout << "Process resumed successfully." << endl;
}

void ChangeDirectory(const string& path) {
    if (SetCurrentDirectory(path.c_str()) == 0) {
        cout << "Failed to change directory." << endl;
    } else {
        cout << "Current directory changed to: " << path << endl;
    }
}
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT && g_ForegroundProcess != NULL) {
        TerminateProcess(g_ForegroundProcess, 0);
        return TRUE;
    }

    return FALSE;
}
void ExecuteBatchFile(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            system(line.c_str());
        }
        file.close();
    } else {
        cout << "Failed to open batch file: " << filename << endl;
    }
}
void ForegroundProcess(const char* filePath) {
    SHELLEXECUTEINFOA shellInfo;
    ZeroMemory(&shellInfo, sizeof(SHELLEXECUTEINFOA));

    shellInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellInfo.hwnd = NULL;
    shellInfo.lpVerb = "open";
    shellInfo.lpFile = filePath;
    shellInfo.lpParameters = NULL;
    shellInfo.lpDirectory = NULL;
    shellInfo.nShow = SW_SHOWNORMAL;

    ShellExecuteExA(&shellInfo);
    WaitForSingleObject(shellInfo.hProcess, INFINITE);
    CloseHandle(shellInfo.hProcess);
}

void BackgroundProcess(const char* filePath) {
    SHELLEXECUTEINFOA shellInfo;
    ZeroMemory(&shellInfo, sizeof(SHELLEXECUTEINFOA));

    shellInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellInfo.hwnd = NULL;
    shellInfo.lpVerb = "open";
    shellInfo.lpFile = filePath;
    shellInfo.lpParameters = NULL;
    shellInfo.lpDirectory = NULL;
    shellInfo.nShow = SW_SHOW;  // ?n c?a s? c?a ti?n trình

    if (ShellExecuteExA(&shellInfo)) {
        std::cout << "File opened in background." << std::endl;
    }
    else {
        std::cerr << "Failed to open file." << std::endl;
    }
}

bool addPath(const std::string& newPath) {
    std::string path = getenv("PATH");
    path += ";" + newPath;  // Th?m du?ng d?n m?i v?o bi?n m?i tru?ng

    int result = _putenv(("PATH=" + path).c_str());

    if (result != 0) {
        std::cerr << "Failed to update the PATH environment variable." << std::endl;
        return false;
    }

    return true;
}

void UpdateTime() {
    int sec_prev = 0;
    char input;
    bool shouldStop = false; // Bi?n di?u ki?n

    while (!shouldStop) {
        string str;

        // Luu t?ng s? giây
        time_t total_seconds = time(0);

        // L?y giá tr? giây, phút và gi?
        struct tm* localTime = localtime(&total_seconds);

        int seconds = localTime->tm_sec;
        int minutes = localTime->tm_min;
        int hours = localTime->tm_hour;

        // In k?t qu?
        if (seconds == sec_prev + 1 || (sec_prev == 59 && seconds == 0)) {
            cout << '\r';
            cout << "Time: " << (hours < 10 ? "0" : "") << hours << ":" << (minutes < 10 ? "0" : "") << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << " " << str;
        }
        sec_prev = seconds;

        // Ki?m tra xem có phím nào du?c nh?n hay không
        if (_kbhit()) {
            input = _getch();
            if (input == 13) { // Nh?n phím Enter d? d?ng l?i
                shouldStop = true;
                cout<<endl;
            }
        }
    }
}

void Get_Date()
{
    // L?y th?i gian hi?n t?i
    std::time_t currentTime = std::time(NULL);

    // Chuy?n d?i th?nh c?u tr?c tm
    std::tm* localTime = std::localtime(&currentTime);

    std::cout << "Date: " << localTime->tm_mday << "/" << localTime->tm_mon + 1 << "/" << localTime->tm_year + 1900 << std::endl;
}

void listofCurrent() {
    char buffer[FILENAME_MAX];
    if (_getcwd(buffer, sizeof(buffer)) == NULL) {
        std::cout << "Failed to retrieve current working directory" << std::endl;
        return;
    }

    std::cout << "Current Working Directory: " << buffer << std::endl;

    system("dir");
}


void readcmd(const string& command) {
    if (command.compare("help") == 0) {
        help();
    } else if (command.compare("time") == 0) {
        cout<<setw(40)<<"Press Enter to stop!";
		UpdateTime();
    } else if (command.compare("date") == 0) {
        Get_Date();
    } else if (command.compare("path") == 0) {
         extern char** environ;  // M?ng ch?a c?c bi?n m?i tru?ng

    // Duy?t qua m?ng bi?n m?i tru?ng
    	for (int i = 0; environ[i] != nullptr; ++i) {
        	std::string envVar(environ[i]);
        	std::cout << envVar << std::endl;
    	}
    } else if (command.compare("dir") == 0) {
        listofCurrent();
    } else if (command.compare("cd") == 0) {
    	string direct;
    	cout<< "Please enter path of the folder you want to change directory: ";
    	cin >> direct;
    	ChangeDirectory(direct);
    } else if (command.compare("list") == 0) {
        ListProcesses();
    } else if (command.compare("kill") == 0) {
    	DWORD id;
    	cout<< "Please enter id of the process you want to kill: ";
    	cin >> id;
 		KillProcess(id);
    } else if (command.compare("stop") == 0) {
    	DWORD id;
    	cout<< "Please enter id of the process you want to stop: ";
    	cin >> id;
 		SuspendProcess(id);
    } else if (command.compare("resume") == 0) {
    	DWORD id;
    	cout<< "Please enter id of the process you want to resume: ";
    	cin >> id;
		ResumeProcess(id);
	} else if (command.compare("readbat") == 0) {
		string filename;
		cout<< "Please enter filename: ";
		cin >> filename;
		ExecuteBatchFile(filename);
	} else if (command.compare("addpath")==0){
		std::string pathToAdd;
		cout<< "Please enter the new path: ";
		cin >> pathToAdd;
		if (addPath(pathToAdd)) {
        std::cout << "Successfully updated the PATH environment variable." << std::endl;
    } else {
        std::cout << "Failed to update the PATH environment variable." << std::endl;
    }
	} else if (command.compare("timer")==0){
		string cmd;
		cout<<"foreground or background? ";
		while(true){
		cin>>cmd;
		if(cmd.compare("foreground")==0){
			ForegroundProcess(timerPath);
			break;
		} else if(cmd.compare("background")==0){
			BackgroundProcess(timerPath);
			break;
			} else cout<<"Please retype the mode: ";	
		}
	}
}
int main() {
    string command;
    cout << "--------------------------------------Welcome to TINY SHELL!------------------------------------------" <<endl;
    cout << "                               Phan Trong Cuong || Tran Tuan Minh                                    "<<endl;
    cout << "-------------------------------Type 'help' for command information!-----------------------------------" << endl;
    while (true) {
    	cout<<"Please enter the command: ";
        cin >> command;
        if(command.compare("exit")==0){ 
	cout << "                                    Thanks for using our shell!                                         "<<endl;
		break;
		}
        else{
        readcmd(command);
    cout << "-----------------------------------------------------------------------------------------------------"<<endl;
        }
    }
    return 0;
}



