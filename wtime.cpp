
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

// Software version (major.minor.path date time)
static const char VERSION[] = "0.0.1 " __DATE__ " " __TIME__;

static void ShowErrorMessage(DWORD errorCode)
{
    char *buffer = nullptr;

    FormatMessageA
    (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &buffer, 0, nullptr
    );

    if (buffer != nullptr)
    {
        printf_s("%s", buffer);
        LocalFree(buffer);
    }
}

static unsigned long long SubtractTime(FILETIME ftA, FILETIME ftB)
{
    ULARGE_INTEGER a, b;

    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;

    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;

    return a.QuadPart - b.QuadPart;
}

static double ToMilliseconds(unsigned long long t)
{
    unsigned long long TICKS_PER_MS = 10000;
    return (double)t / (double)TICKS_PER_MS;
}

static double ToMilliseconds(FILETIME ft)
{
    ULARGE_INTEGER r;

    r.LowPart = ft.dwLowDateTime;
    r.HighPart = ft.dwHighDateTime;

    return ToMilliseconds(r.QuadPart);
}

static void PrintTime(const char *prefix, double milliseconds)
{
    int d = 0, h = 0, m = 0, s = 0;
    auto ms = static_cast<unsigned long long>(milliseconds);

    if (ms >= 1000)
    {
        s = (int)(ms / 1000);   // seconds
        ms %= 1000;             // remaining milliseconds

        m = s / 60;             // minutes
        s %= 60;                // remaining seconds

        h = m / 60;             // hours
        m %= 60;                // remaining minutes

        d = h / 24;             // days
        h %= 24;                // remaining hours
    }

    printf_s("%s % 4dd % 2dh % 2dm % 2ds % 3dms\n", prefix, d, h, m, s, (int)ms);
}

static void ShowExecutionTime(HANDLE hProcess)
{
    FILETIME creationTime = { 0 }, exitTime = { 0 };
    FILETIME kernelTime = { 0 }, userTime = { 0 };

    if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        ShowErrorMessage(GetLastError());
        return;
    }

    ULARGE_INTEGER r;
    r.QuadPart = SubtractTime(exitTime, creationTime);

    putchar('\n');
    putchar('\n');

    double time = ToMilliseconds(r.QuadPart);
    PrintTime("real:", time);

    time = ToMilliseconds(userTime);
    PrintTime("user:", time);

    time = ToMilliseconds(kernelTime);
    PrintTime("sys :", time);
}

static void RunProgram(const std::string &program, const std::string &arguments)
{
    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};

    si.cb = (DWORD)sizeof(si);
    si.wShowWindow = SW_NORMAL;
    si.dwFlags |= STARTF_USESHOWWINDOW;

    std::string commandLine(program + " " + arguments);
    char *cmd = const_cast<char*>(commandLine.c_str());

    BOOL oK = CreateProcessA
    (
        nullptr, cmd, nullptr, nullptr, FALSE, CREATE_NEW_PROCESS_GROUP,
        nullptr, nullptr, &si, &pi
    );

    if (oK)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        ShowExecutionTime(pi.hProcess);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        DWORD err = GetLastError();
        ShowErrorMessage(err);
    }
}

static void ShowHelp()
{
    {
        printf_s("Usage:\n");
        printf_s("  wtime command [arguments ...]\n\n");
    }
    {
        printf_s("VERSION:\n");
        printf_s("  %s\n\n", VERSION);
    }
    {
        printf_s("OUTPUT TIPS:\n");
        printf_s("  real: the amount of time between the the process has exit and created\n\n");

        printf_s("  user: amount of time that the process has executed in user mode.\n");
        printf_s("        This value can exceed the amount of real time if the process \n");
        printf_s("        executes across multiple CPU cores.\n\n");

        printf_s("  sys : amount of time that the process has executed in kernel mode.\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        ShowHelp();
        return EXIT_FAILURE;
    }

    std::string arguments;
    arguments.reserve(1024);

    for (int i = 2; i < argc; ++i)
    {
        arguments.append(argv[i]);
        arguments.append(i < argc - 1 ? " " : "");
    }

    RunProgram(argv[1], arguments);
    return EXIT_SUCCESS;
}
