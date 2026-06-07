#include "defs.hpp"
#include <wdmsec.h>
#include "routines/routines.hpp"
#include "util/clean.hpp"

UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\MemDrv");
UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Global\\MemDrv");

void DriverUnload(PDRIVER_OBJECT driverObject) {
    IoDeleteSymbolicLink(&SymbolicLink);
    IoDeleteDevice(driverObject->DeviceObject);
    DbgPrint("[+] Driver Unloaded\n");
}

NTSTATUS IoControl(PDEVICE_OBJECT deviceObject, PIRP irp) {
    UNREFERENCED_PARAMETER(deviceObject);

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG inputLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputLength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    PVOID buffer = irp->AssociatedIrp.SystemBuffer;

    if (!buffer) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        __try {
            switch (controlCode) {
            case IOCTL_MEMORY_READ: {
                if (inputLength < sizeof(MEMORY_REQUEST) || outputLength < sizeof(MEMORY_REQUEST)) {
                    status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
                PMEMORY_REQUEST request = (PMEMORY_REQUEST)buffer;

                // Security: Probe the nested user-mode buffer
                if (request->Buffer && request->Size > 0) {
                    ProbeForWrite(request->Buffer, (SIZE_T)request->Size, 1);
                }

                request->Status = routines::ReadProcessMemoryKernel(request->ProcessId, request->Address, request->Buffer, request->Size);
                status = request->Status;
                break;
            }
            case IOCTL_MEMORY_WRITE: {
                if (inputLength < sizeof(MEMORY_REQUEST) || outputLength < sizeof(MEMORY_REQUEST)) {
                    status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
                PMEMORY_REQUEST request = (PMEMORY_REQUEST)buffer;

                // Security: Probe the nested user-mode buffer
                if (request->Buffer && request->Size > 0) {
                    ProbeForRead(request->Buffer, (SIZE_T)request->Size, 1);
                }

                request->Status = routines::WriteProcessMemoryKernel(request->ProcessId, request->Address, request->Buffer, request->Size);
                status = request->Status;
                break;
            }
            case IOCTL_MODULE_BASE: {
                if (inputLength < sizeof(MODULE_REQUEST) || outputLength < sizeof(MODULE_REQUEST)) {
                    status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
                PMODULE_REQUEST request = (PMODULE_REQUEST)buffer;

                // Security: Ensure null termination for name string
                request->ModuleName[255] = L'\0';

                request->Status = routines::GetModuleBaseAddress(request->ProcessId, request->ModuleName, &request->BaseAddress);
                status = request->Status;
                break;
            }
            case IOCTL_PROCESS_ID: {
                if (inputLength < sizeof(PID_REQUEST) || outputLength < sizeof(PID_REQUEST)) {
                    status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
                PPID_REQUEST request = (PPID_REQUEST)buffer;

                // Security: Ensure null termination for name string
                request->ProcessName[255] = L'\0';

                request->Status = routines::GetProcessIdByName(request->ProcessName, &request->ProcessId);
                status = request->Status;
                break;
            }
            default:
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            DbgPrint("[-] Exception in IoControl: 0x%X\n", status);
        }
    }

    irp->IoStatus.Status = status;
    irp->IoStatus.Information = (status == STATUS_SUCCESS) ? outputLength : 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS CreateClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
    UNREFERENCED_PARAMETER(deviceObject);
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(registryPath);

    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = nullptr;

    // Use a strict SDDL to prevent unauthorized access
    // RW for System and Administrators only
    UNICODE_STRING sddl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

    status = IoCreateDeviceSecure(driverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &sddl, nullptr, &deviceObject);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[-] Failed to create secure device: 0x%X\n", status);
        return status;
    }

    status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        return status;
    }

    driverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
    driverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
    driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
    driverObject->DriverUnload = DriverUnload;

    DbgPrint("[+] Driver Loaded\n");

    return STATUS_SUCCESS;
}