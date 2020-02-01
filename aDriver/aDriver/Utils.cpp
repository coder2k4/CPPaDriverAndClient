#include "Utils.h"

extern uint64_t offset1;

//�������� ������� ������ �������� �� PID
uint64_t handle_get_base_address(int pid)
{
	//������� ���������� process � ������� ����������
	PEPROCESS process = nullptr;
	//�������� status (�����) �� ������� � ���� ��� ������ �������� � �������
	//��������� ��������� �� ��������� EPROCESS �������� process.
	NTSTATUS status = PsLookupProcessByProcessId(HANDLE(pid), &process);

	//���� �� �������� �������
	if (!NT_SUCCESS(status))
	{
		return 0;
	}

	//�������� ������� ������ ��������
	const auto base_address = uint64_t(PsGetProcessSectionBaseAddress(process));
	ObDereferenceObject(process); //���������� ������ �� �������

	return base_address; //���������� ������� ������
}


/*
 * �������� �������� Dword �� �������
 */
NTSTATUS getRegDword(IN ULONG  RelativeTo, IN PWSTR  Path, IN PWSTR ParameterName, IN OUT PULONG ParameterValue)
{
	//������������� ���������� � �������� �������
	RTL_QUERY_REGISTRY_TABLE  paramTable[2];

	//���� �� ���� �������� ��������� - ���������� ������
	if ((NULL == Path) || (NULL == ParameterName) || (NULL == ParameterValue)) {
		return STATUS_INVALID_PARAMETER;
	}

	//��������� ���� ������ ������, �������� ��������� �� ���� � ����� � ������ ��� ����������.
	RtlZeroMemory(paramTable, sizeof(paramTable));

	
	ULONG defaultValue = *ParameterValue;

	paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	paramTable[0].Name = ParameterName;
	paramTable[0].EntryContext = ParameterValue;
	paramTable[0].DefaultType = REG_DWORD;
	paramTable[0].DefaultData = &defaultValue;
	paramTable[0].DefaultLength = sizeof(ULONG); //sizeof(ULONGLONG);

	//��������� ���������� ������� ����������� ��������� �������� �� ��������� ������� � ������� ������ ������.
	NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
		Path,
		&paramTable[0],
		NULL,
		NULL);

	
	if (status != STATUS_SUCCESS) {
		//DbgPrintEx(0, 0, "FAILED w/status=%x\n", status);
	}

	return status;
}

/*
 * �������� �������� Qword �� �������
 */
NTSTATUS getRegQword(IN ULONG  RelativeTo, IN PWSTR  Path, IN PWSTR ParameterName, IN OUT PULONGLONG ParameterValue)
{
	RTL_QUERY_REGISTRY_TABLE  paramTable[2]; 

	if ((NULL == Path) || (NULL == ParameterName) || (NULL == ParameterValue)) {
		return STATUS_INVALID_PARAMETER;
	}

	RtlZeroMemory(paramTable, sizeof(paramTable));

	ULONGLONG defaultValue = *ParameterValue;

	paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	paramTable[0].Name = ParameterName;
	paramTable[0].EntryContext = ParameterValue;
	paramTable[0].DefaultType = REG_QWORD;
	paramTable[0].DefaultData = &defaultValue;
	paramTable[0].DefaultLength = sizeof(ULONGLONG);

	NTSTATUS status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
		Path,
		&paramTable[0],
		NULL,
		NULL);

	if (status != STATUS_SUCCESS) {
		//DbgPrintEx(0, 0, "FAILED w/status=%x\n", status);
	}

	return status;
}



/*
 * ������������� ��������� �� ������
 */
bool listen(int tpid, uint64_t packetAdress, const Packet& out, PEPROCESS destProcess)
{
	PEPROCESS srcProcess = nullptr;

	//�������� status (�����) �� ������� � ���� ��� ������ �������� � �������
	//��������� ��������� �� ��������� EPROCESS �������� process.
	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(tpid), &srcProcess)))
	{
		return 0;
	}

	SIZE_T retsz = 0;

	//�������� ������ �� ������ ������ � out.
	/*
	 *	srcProcess - ������� ��������� �� ID (����������) // ������ �� �������� - �������
	 *	(void*)(base + offset1) - �������� ������ (��� ������) // ������ �� �������� - ������� ������
	 *	destProcess - ������� ������� - ��� ������� // ������ �� ���������� - �������
	 *	(void*)&out - ���� ����� ����������� ��� ����� // ������ �� ���������� - ������� ������
	 *	sizeof(Packet) - ������ ������������ ������ // ������ �������
	 *	KernelMode - ����� ����	// �����
	 *	&retsz - ������ // ������������ ������
	 */
	NTSTATUS ret = MmCopyVirtualMemory(srcProcess, (void*)(packetAdress), destProcess, (void*)&out, sizeof(Packet), KernelMode, &retsz);

	//ObDereferenceObject(destProcess);
	//������� ���������
	ObDereferenceObject(srcProcess);

	if (ret == STATUS_SUCCESS)
		return 1;
	else
		return 0;
}


/*
 * ������������� ������� ��� ������ � ������ �� ������ � ��������� ������
 */
void handlememory_meme(const Packet& in)
{
	PEPROCESS destProcess = nullptr;
	PEPROCESS srcProcess = nullptr;

    //�������� ��������� �� ������� �� dest_pid
	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(in.dest_pid), &destProcess)))
		return;
	//�������� ��������� �� ������� �� source_pid
	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(in.source_pid), &srcProcess)))
		return;

	SIZE_T retsz = 0;

	//DbgPrintEx(0, 0, ">handling memory operation..\n");
	//DbgPrintEx(0, 0, "Source pid: %i,@: 0x%p\n",in.source_pid,in.source);
	//DbgPrintEx(0, 0, "Dest   pid: %i,@: 0x%p\n", in.dest_pid, in.destination);
	//DbgPrintEx(0, 0, "size of data: %i\n",in.size);

	MmCopyVirtualMemory(srcProcess, (void*)in.source, destProcess, (void*)in.destination, in.size, KernelMode, &retsz);

	//if (status == STATUS_SUCCESS)
		//DbgPrintEx(0, 0, ">success\n");
	//else
		//DbgPrintEx(0, 0, ">failed\n");

	ObDereferenceObject(destProcess);
	ObDereferenceObject(srcProcess);
}


/*
 * ������� ������������� ������ ������
 */
void writeopcode(int tpid, uint32_t opcode, uint64_t packetAdress, PEPROCESS srcProcess)
{
	PEPROCESS destProcess = nullptr;
	//PEPROCESS srcProcess = PsGetCurrentProcess();

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(tpid), &destProcess)))
	{
		return;
	}

	SIZE_T retsz = 0;

	MmCopyVirtualMemory(srcProcess, (void*)&opcode, destProcess, (void*)(packetAdress), sizeof(uint32_t), KernelMode, &retsz);

	ObDereferenceObject(destProcess);
	//ObDereferenceObject(srcProcess);
}


/*
 * ������� ��� ��������� �������� ������
 */
void handle_base_packet(const Packet& in, int tpid, PEPROCESS srcProcess)
{
	static uint64_t base = handle_get_base_address(in.dest_pid);

	//DbgPrintEx(0, 0, "->got base: 0x%p\n", base);

	PEPROCESS destProcess = nullptr;
	//PEPROCESS srcProcess = PsGetCurrentProcess();

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(tpid), &destProcess)))
	{
		return;
	}

	SIZE_T retsz = 0;

	MmCopyVirtualMemory(srcProcess, (void*)&base, destProcess, (void*)in.destination, sizeof(uint64_t), KernelMode, &retsz);

	ObDereferenceObject(destProcess);
	//ObDereferenceObject(srcProcess);
}

//��������� ������
void wipepacket(Packet& in) noexcept
{
	in.destination = 0;
	in.dest_pid = 0;
	in.opcode = OP_INVALID;
	in.size = 0;
	in.source = 0;
	in.source_pid = 0;
}