#include <windows.h>
#include <iostream>

int main() {

	const char* dllPath = "C:\\Path\\To\\Your\\DLL.dll";
	DWORD processId = 0; // Process ID of the target process

	//open handle of process 
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (hProcess == NULL) {
		std::cerr << "Failed to open process" << std::endl;
		return 1;
	}

	//allocate memory in the target process
	LPVOID allocMemory = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (allocMemory == NULL) {
		std::cerr << "Failed to allocate memory in the target process" << std::endl;
		CloseHandle(hProcess);
		return 1;
	}

	//write the DLL path to the allocated mem
	if (!WriteProcessMemory(hProcess, allocMemory, dllPath, strlen(dllPath) + 1, NULL)) {
		std::cerr << "Failed to write memory: " << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, allocMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return 1;
	}

	//create a remote thread in the target process
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, allocMemory, 0, NULL);
	if (hThread == NULL) {
		std::cerr << "Failed to create remote thread: " << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, allocMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE);

	//cleanup
	VirtualFreeEx(hProcess, allocMemory, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	std::cout << "DLL injected successfully" << std::endl;
	return 0;

}