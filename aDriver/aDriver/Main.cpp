#include <ntddk.h>
#include "cImports.h"
#include "Utils.h"

//#define ABSOLUTE(wait) (wait)
//#define RELATIVE(wait) (-(wait))
//#define NANOSECONDS(nanos) \
//(((signed __int64)(nanos)) / 100L)
//#define MICROSECONDS(micros) \
//(((signed __int64)(micros)) * NANOSECONDS(1000L))
//#define MILLISECONDS(milli) \
//(((signed __int64)(milli)) * MICROSECONDS(1000L))
//#define SECONDS(seconds) \
//(((signed __int64)(seconds)) * MILLISECONDS(1000L))


uint64_t offset1 = 0x0, offset2 = 0x0, offset3 = 0x0;// �������� �������� ��� ������ ������������ (��� ��� ����� ��� ����� P � �������� � �������) ��� 3 ���������
KTIMER timer1, timer2, timer3; //����� ���������� ����

//��������� ��� �����
void NTAPI thread1(void*)
{
	//�������������� ����������� ������ ������� ����.
	KeInitializeTimerEx(&timer1, TIMER_TYPE::SynchronizationTimer); // 1ms
	const LARGE_INTEGER due = { 0 };
	/*
	 *  KeSetTimerEx ������������� ���������� ��� ������������� ��������, � ������� ������ ������� ������ ���� ����������
	 *  � ���������� ���������, ������������� ������������� ������������ CustomTimerDpc, ������� ����� ����������� ��
	 *  ��������� ����� ���������, � ��� ������������� ������������� ������������� �������� ��� �������.
	 */
	KeSetTimerEx(&timer1, due, 1, nullptr);

	unsigned long client_pid = 0; // ID �������� clientV2
	bool b_gotBaseFlag = false; // ���� ��������� �������� ������ ��������
	uint64_t base = 0; // ��� ������� ������ ��������
	Packet p{}; // ������� ���������� p ��� ������������

	PEPROCESS destProcess = PsGetCurrentProcess(); //���������� ��������� �� ������� �������� ������.

	//DbgPrintEx(0, 0, "Thread #1 START");
	
	while (true)
	{
		//������������ ��������� ������� ����� � ��������� ��������, ���� ������ ������ ���������� �� �����
		//���������� � ���������� ��������� ��� (�������������), ���� ����� �������� �� �������.
		//���� * Timeout = 0, ��������� ������������ ��� ��������.
		KeWaitForSingleObject(&timer1, Executive, KernelMode, FALSE, 0);

		//�������� �������� �� ������� �� ����� pid  � ��������� pid ����������
		getRegDword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"pid", &client_pid);
		//�������� �������� �� ������� �� ����� offset1 � ��������� offset1 ����������
		getRegQword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"offset1", &offset1);


		//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
		//DbgPrintEx(0, 0, "THREAD1 ->pid - %i, offset1 - 0x%p", client_pid, offset1);

		//�������� ��������� �� PID � OFFSET ��������
		if (client_pid > 0 && offset1)
		{
			if (!b_gotBaseFlag) //���� �� �������� ������ �� �������, ���������� ���� � TRUE
			{
				b_gotBaseFlag = true; //������������� ���� � ������������� ��������

				base = handle_get_base_address(client_pid); //�������� ������� ������

				//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
				//DbgPrintEx(0, 0, "found um client - pid: %i - base: 0x%p - offset1: 0x%p\n", client_pid, base, offset1);
			}

			if (base) // ���� ������� ����, �� ...
			{
				if (listen(client_pid, base + offset1, p, destProcess)) //������ ����� � ���������� p �� �������
				{
					if (p.opcode == OP_READ)
					{
						//DbgPrintEx(0, 0, "THREAD1 ->opcode READ: %i\n", p.opcode);
						handlememory_meme(p); //������ �� ������
						writeopcode(client_pid, OP_DONE, base + offset1, destProcess);
					}

					if (p.opcode == OP_WRITE)
					{
						//DbgPrintEx(0, 0, "THREAD1 ->opcode WRITE: %i\n", p.opcode);
						handlememory_meme(p); //���������� � ������
						writeopcode(client_pid, OP_DONE, base + offset1, destProcess);
					}

					if (p.opcode == OP_BASE)
					{
						//DbgPrintEx(0, 0, "THREAD1 ->opcode BASE: %i\n", p.opcode);
						//DbgPrintEx(0, 0, "THREAD1 ->pid: %i, dest: 0x%p\n", p.dest_pid, p.destination);
						handle_base_packet(p, client_pid, destProcess); //��������� �������� ������
						writeopcode(client_pid, OP_DONE, base + offset1, destProcess);
					}
				}
			}
		}
		else
		{
			b_gotBaseFlag = false;
		}
	}
	KeCancelTimer(&timer1);
}

void NTAPI thread2(void*)
{
	//�������������� ����������� ������ ������� ����.
	KeInitializeTimerEx(&timer2, TIMER_TYPE::SynchronizationTimer); // 1ms
	const LARGE_INTEGER due = { 0 };
	/*
	 *  KeSetTimerEx ������������� ���������� ��� ������������� ��������, � ������� ������ ������� ������ ���� ����������
	 *  � ���������� ���������, ������������� ������������� ������������ CustomTimerDpc, ������� ����� ����������� ��
	 *  ��������� ����� ���������, � ��� ������������� ������������� ������������� �������� ��� �������.
	 */
	KeSetTimerEx(&timer2, due, 1, nullptr);

	unsigned long client_pid = 0; // ID �������� clientV2
	bool b_gotBaseFlag = false; // ���� ��������� �������� ������ ��������
	uint64_t base = 0; // ��� ������� ������ ��������
	Packet p{}; // ������� ���������� p ��� ������������	

	PEPROCESS destProcess = PsGetCurrentProcess(); //���������� ��������� �� ������� �������� ������.

	//DbgPrintEx(0, 0, "Thread #2 START");
	
	while (true)
	{
		//������������ ��������� ������� ����� � ��������� ��������, ���� ������ ������ ���������� �� �����
		//���������� � ���������� ��������� ��� (�������������), ���� ����� �������� �� �������.
		//���� * Timeout = 0, ��������� ������������ ��� ��������.
		KeWaitForSingleObject(&timer2, Executive, KernelMode, FALSE, 0);

		//�������� �������� �� ������� �� ����� pid  � ��������� pid ����������
		getRegDword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"pid", &client_pid);
		//�������� �������� �� ������� �� ����� offset1 � ��������� offset1 ����������
		getRegQword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"offset2", &offset2);


		//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
		//DbgPrintEx(0, 0, "THREAD2 ->pid - %i, offset2 - 0x%p", client_pid, offset2);

		//�������� ��������� �� PID � OFFSET ��������
		if (client_pid > 0 && offset2)
		{
			if (!b_gotBaseFlag) //���� �� �������� ������ �� �������, ���������� ���� � TRUE
			{
				b_gotBaseFlag = true; //������������� ���� � ������������� ��������

				base = handle_get_base_address(client_pid); //�������� ������� ������

				//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
				//DbgPrintEx(0, 0, "found um client - pid: %i - base: 0x%p - offset1: 0x%p\n", client_pid, base, offset2);
			}

			if (base) // ���� ������� ����, �� ...
			{
				if (listen(client_pid, base + offset2, p, destProcess)) //������ ����� � ���������� p �� �������
				{
					if (p.opcode == OP_READ)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode READ: %i\n", p.opcode);
						handlememory_meme(p); //������ �� ������
						writeopcode(client_pid, OP_DONE, base + offset2, destProcess);
					}

					if (p.opcode == OP_WRITE)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode WRITE: %i\n", p.opcode);
						handlememory_meme(p); //���������� � ������
						writeopcode(client_pid, OP_DONE, base + offset2, destProcess);
					}

					if (p.opcode == OP_BASE)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode BASE: %i\n", p.opcode);
						//DbgPrintEx(0, 0, "THREAD2 ->pid: %i, dest: 0x%p\n", p.dest_pid, p.destination);
						handle_base_packet(p, client_pid, destProcess); //��������� �������� ������
						writeopcode(client_pid, OP_DONE, base + offset2, destProcess);
					}
				}
			}
		}
		else
		{
			b_gotBaseFlag = false;
		}
	}
	KeCancelTimer(&timer2);
}

void NTAPI thread3(void*)
{
	//�������������� ����������� ������ ������� ����.
	KeInitializeTimerEx(&timer3, TIMER_TYPE::SynchronizationTimer); // 1ms
	const LARGE_INTEGER due = { 0 };
	/*
	 *  KeSetTimerEx ������������� ���������� ��� ������������� ��������, � ������� ������ ������� ������ ���� ����������
	 *  � ���������� ���������, ������������� ������������� ������������ CustomTimerDpc, ������� ����� ����������� ��
	 *  ��������� ����� ���������, � ��� ������������� ������������� ������������� �������� ��� �������.
	 */
	KeSetTimerEx(&timer3, due, 1, nullptr);

	unsigned long client_pid = 0; // ID �������� clientV2
	bool b_gotBaseFlag = false; // ���� ��������� �������� ������ ��������
	uint64_t base = 0; // ��� ������� ������ ��������
	Packet p{}; // ������� ���������� p ��� ������������	

	PEPROCESS destProcess = PsGetCurrentProcess(); //���������� ��������� �� ������� �������� ������.

	//DbgPrintEx(0, 0, "Thread #3 START");

	while (true)
	{
		//������������ ��������� ������� ����� � ��������� ��������, ���� ������ ������ ���������� �� �����
		//���������� � ���������� ��������� ��� (�������������), ���� ����� �������� �� �������.
		//���� * Timeout = 0, ��������� ������������ ��� ��������.
		KeWaitForSingleObject(&timer3, Executive, KernelMode, FALSE, 0);

		//�������� �������� �� ������� �� ����� pid  � ��������� pid ����������
		getRegDword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"pid", &client_pid);
		//�������� �������� �� ������� �� ����� offset1 � ��������� offset1 ����������
		getRegQword(0, L"\\Registry\\Machine\\SOFTWARE\\WowAesdt", L"offset3", &offset3);


		//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
		//DbgPrintEx(0, 0, "THREAD3 ->pid - %i, offset3 - 0x%p", client_pid, offset3);

		//�������� ��������� �� PID � OFFSET ��������
		if (client_pid > 0 && offset3)
		{
			if (!b_gotBaseFlag) //���� �� �������� ������ �� �������, ���������� ���� � TRUE
			{
				b_gotBaseFlag = true; //������������� ���� � ������������� ��������

				base = handle_get_base_address(client_pid); //�������� ������� ������

				//���������� ������ � �������� ����, ���� ��������� ���� ������� ���������.
				//DbgPrintEx(0, 0, "found um client - pid: %i - base: 0x%p - offset1: 0x%p\n", client_pid, base, offset3);
			}

			if (base) // ���� ������� ����, �� ...
			{
				if (listen(client_pid, base + offset3, p, destProcess)) //������ ����� � ���������� p �� �������
				{
					if (p.opcode == OP_READ)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode READ: %i\n", p.opcode);
						handlememory_meme(p); //������ �� ������
						writeopcode(client_pid, OP_DONE, base + offset3, destProcess);
					}

					if (p.opcode == OP_WRITE)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode WRITE: %i\n", p.opcode);
						handlememory_meme(p); //���������� � ������
						writeopcode(client_pid, OP_DONE, base + offset3, destProcess);
					}

					if (p.opcode == OP_BASE)
					{
						//DbgPrintEx(0, 0, "THREAD2 ->opcode BASE: %i\n", p.opcode);
						//DbgPrintEx(0, 0, "THREAD2 ->pid: %i, dest: 0x%p\n", p.dest_pid, p.destination);
						handle_base_packet(p, client_pid, destProcess); //��������� �������� ������
						writeopcode(client_pid, OP_DONE, base + offset3, destProcess);
					}
				}
			}
		}
		else
		{
			b_gotBaseFlag = false;
		}
	}
	KeCancelTimer(&timer3);
}
/*
 * ����� ����� � �������� ��������
 */
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	// These are invalid for mapped drivers.
	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);


	//������������ ������ � ���������� �������, ������� ��������� ��� �������� APC � ������ ���� � ������� �����.
	/*
	 * ����������� ����� ��������� (APC) - ��� �������, ������� ����������� ���������� � ��������� ������������� ������.
	 * ����� APC �������� � ������� � �����, ������� ������ ����������� ����������.
	 * ��������������� �������� APC ���������� APC � ������ ����. APC, ��������������� �����������,
	 * ���������� APC ����������������� ������.
	 */
	KeEnterGuardedRegion();

	//�������� ��� ������ ���������� ���� � ���������� ��������� �� ���������� ����.
	PWORK_QUEUE_ITEM WorkItem1 = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
	if (!WorkItem1) return STATUS_UNSUCCESSFUL;

	//�������������� ������� ������� ������� � ��������������� ���������� ���������� � ������������� ��������� ������,
	//������� �������� � ������� ��� ����������, ����� ��������� ������� ����� �������� ��������.
	//�������� ������ ���������� ����� �����
	ExInitializeWorkItem(WorkItem1, thread1, WorkItem1);

	//��������� ������ ������� ������� � �������, �� ������� ��������� ������� ����� ������� ������� �
	//�������� ���������� ������������, ��������������� ���������� �������� ��� ExInitializeWorkItem
	ExQueueWorkItem(WorkItem1, DelayedWorkQueue);


	//2� �����
	PWORK_QUEUE_ITEM WorkItem2 = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
	if (!WorkItem2) return STATUS_UNSUCCESSFUL;
	ExInitializeWorkItem(WorkItem2, thread2, WorkItem2);
	ExQueueWorkItem(WorkItem2, DelayedWorkQueue);
	//3� �����
	PWORK_QUEUE_ITEM WorkItem3 = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
	if (!WorkItem3) return STATUS_UNSUCCESSFUL;
	ExInitializeWorkItem(WorkItem3, thread3, WorkItem3);
	ExQueueWorkItem(WorkItem3, DelayedWorkQueue);

	
	//KeLeaveGuardedRegion - �������� ������ ������
	KeLeaveGuardedRegion();

	//�������� �������� ������ �����
	return STATUS_SUCCESS;
}


