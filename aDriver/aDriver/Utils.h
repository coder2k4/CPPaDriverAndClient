#pragma once
#include "cImports.h"


//������� ��������� ����� �������

struct Packet
{
	int opcode = 0; // ���������� �������
	uint64_t source = 0; // 
	uint64_t destination = 0; // ������ ������ ��� ���� ����� ������������� ������

	uint32_t source_pid = 0; // id �������� ������ �������� ������
	uint32_t dest_pid = 0; // id �������� ���� ����� �� ��������/�������

	uint32_t size = 0; // ������ ����������� ��� ������������ ������
};

enum opcodes // ������������ ����� ����� ��� ���������� ���������
{
	OP_INVALID = 0,
	OP_READ = 1,
	OP_WRITE = 2,
	OP_BASE = 3,
	OP_DONE = 9
};


/*
 *�������� ������� ������ �������� �� PID
 */
uint64_t handle_get_base_address(int pid); 

/*
 * �������� �������� Dword �� �������
 */
NTSTATUS getRegDword(IN ULONG  RelativeTo, IN PWSTR  Path, IN PWSTR ParameterName, IN OUT PULONG ParameterValue);

/*
 * �������� �������� Dword �� �������
 */
NTSTATUS getRegQword(IN ULONG  RelativeTo, IN PWSTR  Path, IN PWSTR ParameterName, IN OUT PULONGLONG ParameterValue);

/*
 * ������������� ��������� �� ������
 */
bool listen(int tpid, uint64_t packetAdress, const Packet& out, PEPROCESS destProcess);



/*
 * ������������� ������� ��� ������ � ������ �� ������ � ��������� ������
 */
void handlememory_meme(const Packet& in);



/*
 * ������� ������������� ������ ������
 */
void writeopcode(int tpid, uint32_t opcode, uint64_t packetAdress, PEPROCESS srcProcess);



/*
 * ������� ��� ��������� �������� ������
 */
void handle_base_packet(const Packet& in, int tpid, PEPROCESS srcProcess);



//��������� ������
void wipepacket(Packet& in) noexcept;