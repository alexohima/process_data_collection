#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
using namespace std;
ofstream myfile;

//https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes?redirectedfrom=MSDN
typedef long long int64_t;
typedef unsigned long long uint64_t;

/// time convert
static uint64_t file_time_2_utc(const FILETIME *ftime)
{
  LARGE_INTEGER li;

  li.LowPart = ftime->dwLowDateTime;
  li.HighPart = ftime->dwHighDateTime;
  return li.QuadPart;
}

// get CPU number
static int get_processor_number()
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return (int)info.dwNumberOfProcessors;
}

int get_cpu_usage(DWORD pid)
{
  static int processor_count_ = -1;
  static int64_t last_time_ = 0;
  static int64_t last_system_time_ = 0;

  FILETIME now;
  FILETIME creation_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  int64_t system_time;
  int64_t time;
  int64_t system_time_delta;
  int64_t time_delta;

  int cpu = -1;

  if (processor_count_ == -1)
  {
    processor_count_ = get_processor_number();
  }

  GetSystemTimeAsFileTime(&now);

  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
  {
    // can not find the process
    wcout<<"can not find the process";
    exit(EXIT_FAILURE);
  }
  system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processor_count_;
  time = file_time_2_utc(&now);

  if ((last_system_time_ == 0) || (last_time_ == 0))
  {
    last_system_time_ = system_time;
    last_time_ = time;
    return get_cpu_usage(pid);
  }

  system_time_delta = system_time - last_system_time_;
  time_delta = time - last_time_;

  if (time_delta == 0)
  {
    return get_cpu_usage(pid);
  }

  cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
  last_system_time_ = system_time;
  last_time_ = time;

  myfile << cpu <<",";
  wcout << "CPU= " << cpu << "%" << "\n"<< "\n";
  return cpu;

}
// ref: http://www.cppblog.com/cppopp/archive/2012/08/24/188102.aspx
void print_working_set(HANDLE hProcess)
{
    SIZE_T  dwMin, dwMax;
    if (!GetProcessWorkingSetSize(hProcess, &dwMin, &dwMax))
    {
        printf("GetProcessWorkingSetSize failed (%d)\n",
            GetLastError());
    }
    else{
    printf("Minimum working set: %lu KB\n", dwMin/1024);
    printf("Maximum working set: %lu KB\n", dwMax/1024);}
    myfile << dwMin/1024 <<",";
    myfile << dwMax/1024 <<",";
}
void print_open_handles(HANDLE hProcess)
{
    DWORD HandleCount = 0;
    GetProcessHandleCount(hProcess, &HandleCount);
    wcout<<"Handle Count="<< HandleCount<<"\n";
    myfile << HandleCount <<"," << "\n";
}
void start_process( TCHAR *location ,int seconds)
{
    int cpu, count=0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    PDWORD openHandles;

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        location,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    }
    //GET CPU USAGE
    wcout<<" pid="<<pi.dwProcessId;

    do {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(pi.hProcess, &exitCode) == FALSE)
            wcout<<"GetExitCodeProcess failure";

        if (exitCode != STILL_ACTIVE)
            break;

        //GET CPU USAGE
        cpu = get_cpu_usage(pi.dwProcessId);
        //GET WORKING SET
        print_working_set(pi.hProcess);
        //GET OPEN HANDLES
        print_open_handles(pi.hProcess);

        Sleep(seconds*1000);

        } while (true);

    wcout<<"Closed process";

    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close the process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

int _tmain(int argc, _TCHAR* argv[])
{
    myfile.open("log.csv", ios::out | ios::app);
    myfile << "CPU" <<"," << "MIN WORKING SET"<<"," << "MAX WORKING SET"<<"," << "OPEN HANDLES"<<"," << "\n";

    start_process(argv[1],atoi(argv[2]));

    myfile.close();

    exit(EXIT_SUCCESS);
}
