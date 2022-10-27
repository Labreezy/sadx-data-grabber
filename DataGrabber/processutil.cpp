#include "pch.h"
#include <TlHelp32.h>
#include <processthreadsapi.h>
#include <vector>
#include <thread>
#include <chrono>

bool SuspendAllThreads() {
    HANDLE thread_snap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;
    DWORD PID = GetCurrentProcessId();
    // take a snapshot of all running threads
    thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (thread_snap == INVALID_HANDLE_VALUE) {
        printf("Invalid Handle Value");
        return(FALSE);
    }

    // fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);
    
    // retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(thread_snap, &te32)) {
        printf("Thread32First Error");
        CloseHandle(thread_snap);
        return(FALSE);
    }

    // now walk the thread list of the system,
    // and display thread ids of each thread
    // associated with the specified process
    std::vector<DWORD> thread_id_list = std::vector<DWORD>();
    DWORD thisthread_id = GetCurrentThreadId();
    printf("CURRENT THREAD ID: 0x%08x\n", (unsigned int)thisthread_id); //fuck you
    do {
        if (te32.th32OwnerProcessID == PID) {
            DWORD current_thread_id = te32.th32ThreadID;
            if (current_thread_id != thisthread_id) {
                printf("THREAD ID: 0x%08X\n", te32.th32ThreadID);
                thread_id_list.push_back(te32.th32ThreadID);
            }
        }
    } while (Thread32Next(thread_snap, &te32));

    // clean up the snapshot object.
    CloseHandle(thread_snap);
    for (auto it = thread_id_list.begin(); it != thread_id_list.end(); it++) {
        HANDLE threadHnd = OpenThread(THREAD_SUSPEND_RESUME, FALSE, *it);
        SuspendThread(threadHnd);
        CloseHandle(threadHnd);
    }
    return(TRUE);
}

bool ResumeAllThreads() {
    HANDLE thread_snap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;
    DWORD PID = GetCurrentProcessId();
    // take a snapshot of all running threads
    thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (thread_snap == INVALID_HANDLE_VALUE) {
        printf("Invalid Handle Value");
        return(FALSE);
    }

    // fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // retrieve information about the first thread,
    // and exit if unsuccessful
    if (!Thread32First(thread_snap, &te32)) {
        printf("Thread32First Error");
        CloseHandle(thread_snap);
        return(FALSE);
    }

    // now walk the thread list of the system,
    // and display thread ids of each thread
    // associated with the specified process
    std::vector<DWORD> thread_id_list = std::vector<DWORD>();
    DWORD thisthread_id = GetCurrentThreadId();
    printf("CURRENT THREAD ID: 0x%08x\n", (unsigned int)thisthread_id); //fuck you
    do {
        if (te32.th32OwnerProcessID == PID) {
            DWORD current_thread_id = te32.th32ThreadID;
            if (current_thread_id != thisthread_id) {
                printf("THREAD ID: 0x%08X\n", te32.th32ThreadID);
                thread_id_list.push_back(te32.th32ThreadID);
            }
        }
    } while (Thread32Next(thread_snap, &te32));

    // clean up the snapshot object.
    CloseHandle(thread_snap);
    for (auto it = thread_id_list.begin(); it != thread_id_list.end(); it++) {
        HANDLE threadHnd = OpenThread(THREAD_SUSPEND_RESUME, FALSE, *it);
        ResumeThread(threadHnd);
        CloseHandle(threadHnd);
    }
    return (TRUE);
}