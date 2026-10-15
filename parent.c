#include <windows.h>
#include <stdio.h>

DWORD WINAPI ReadOutput(LPVOID param){
    HANDLE pipe_read = (HANDLE)param;
    char ch;
    DWORD bytes_read;

    while(ReadFile(pipe_read, &ch, 1, &bytes_read, NULL) && bytes_read > 0){
        putchar(ch);
        fflush(stdout);
    }
    return 0;
}

int main(void){
    HANDLE p1_read, p1_write;
    HANDLE p2_read, p2_write;
    HANDLE p3_read, p3_write;
    HANDLE reader;
    SECURITY_ATTRIBUTES sa = {0};
    PROCESS_INFORMATION child1 = {0};
    PROCESS_INFORMATION child2 = {0};
    STARTUPINFO si1 = {0};
    STARTUPINFO si2 = {0};

    DWORD written;
    int ch;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&p1_read, &p1_write, &sa, 0) ||
        !CreatePipe(&p2_read, &p2_write, &sa, 0) ||
        !CreatePipe(&p3_read, &p3_write, &sa, 0))
    {
        fprintf(stderr, "CreatePipe error: %lu\n", GetLastError());
        return 1;
    }

    if (!SetHandleInformation(p1_read, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(p1_write, HANDLE_FLAG_INHERIT, 0) ||
        !SetHandleInformation(p2_read, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(p2_write, HANDLE_FLAG_INHERIT, 0) ||
        !SetHandleInformation(p3_read, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(p3_write, HANDLE_FLAG_INHERIT, 0)){

        fprintf(stderr, "SetHandleInformation error: %lu\n", GetLastError());
        return 1;
    }

    if (!SetHandleInformation(p1_read, HANDLE_FLAG_INHERIT, 1) ||
        !SetHandleInformation(p2_write, HANDLE_FLAG_INHERIT, 1)){

        fprintf(stderr, "Enable inheritance error: %lu\n", GetLastError());
        return 1;
    }

    si1.cb = sizeof(si1);
    si1.dwFlags = STARTF_USESTDHANDLES;
    si1.hStdInput = p1_read;
    si1.hStdOutput = p2_write;
    si1.hStdError = p2_write;

    if (!CreateProcess(NULL, "child1.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si1, &child1)){
        fprintf(stderr, "Cannot start child1: %lu\n", GetLastError());
        return 1;
    }

    SetHandleInformation(p1_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(p2_write, HANDLE_FLAG_INHERIT, 0);

    if (!SetHandleInformation(p2_read, HANDLE_FLAG_INHERIT, 1) || !SetHandleInformation(p3_write, HANDLE_FLAG_INHERIT, 1)){
        fprintf(stderr, "Enable inheritance error: %lu\n",
                GetLastError());
        return 1;
    }

    si2.cb = sizeof(si2);
    si2.dwFlags = STARTF_USESTDHANDLES;
    si2.hStdInput = p2_read;
    si2.hStdOutput = p3_write;
    si2.hStdError = p3_write;

    if (!CreateProcess(NULL, "child2.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si2, &child2)){
        fprintf(stderr, "Cannot start child2: %lu\n", GetLastError());
        return 1;
    }

    CloseHandle(p1_read);
    CloseHandle(p2_read);
    CloseHandle(p2_write);
    CloseHandle(p3_write);

    reader = CreateThread(NULL, 0, ReadOutput, p3_read, 0, NULL);

    if (reader == NULL){
        fprintf(stderr, "CreateThread error: %lu\n", GetLastError());
        CloseHandle(p1_write);
        return 1;
    }

    while ((ch = getchar()) != EOF){
        char c = (char)ch;

        if (!WriteFile(p1_write, &c, 1, &written, NULL) || written != 1){
            fprintf(stderr, "WriteFile error: %lu\n", GetLastError());
            break;
        }
    }

    CloseHandle(p1_write);

    WaitForSingleObject(child1.hProcess, INFINITE);
    WaitForSingleObject(child2.hProcess, INFINITE);
    WaitForSingleObject(reader, INFINITE);

    CloseHandle(reader);
    CloseHandle(p3_read);

    CloseHandle(child1.hThread);
    CloseHandle(child1.hProcess);
    CloseHandle(child2.hThread);
    CloseHandle(child2.hProcess);

    return 0;
}
