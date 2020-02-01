#include "Main.h"


//���������� ����������
float g_max_fov = 9999; //������������ �������� ��� ���� �������� ���������, ������������� ����
float g_setup_fov = 10; //������������� ���� ������ AIM


int targetpid = 0; //���� ID ��������
uint64_t aimentity = 0; //�������� ��������� �� ���� ���� � ����
uint64_t tempAimEntyty = 0; // ��������� ����� ��������� �����, �� �������� �� ����� ��������
uint64_t localPlayerEntity; //������ �� ���������� ������



//
//
//
//
//��������� ����� � 3�������� ������������
struct Vector3
{
	float x, y, z;
};


// ������������ ��� ��� ����� ����� � �� ��������
enum opcodes
{
	OP_INVALID = 0,
	OP_READ = 1,
	OP_WRITE = 2,
	OP_BASE = 3,

	OP_DONE = 9
};

// ��������� ������, ������� �������� � ������ ������� ��������
struct Packet
{
	int opcode = 0;
	uint64_t source = 0;
	uint64_t destination = 0;

	uint32_t source_pid = 0;
	uint32_t dest_pid = 0;

	uint32_t size = 0;
};

//���������� ����������
int myPID = 0; // process ID ������ �������
Packet p1, p2, p3; // �������� ������ ��� ������ ������

namespace PTransponder
{
	//�������� ������� ����� PID
	//� ������ ������ �������� ��������� ������� ������ ������ ����������� ����������
	uint64_t get_base(int pid, Packet& p)
	{
		uint64_t var;

		p.dest_pid = pid;
		p.destination = (uint64_t)&var;
		p.opcode = OP_BASE;

		while (p.opcode != OP_DONE)
			Sleep(1);

		p.opcode = OP_INVALID;

		return var;
	}

	//��������� ����� ��� ������ � ������ ����� �������
	template <typename T>
	void write_memory(int pid, T& data, uint64_t addy, Packet& p)
	{
		p.dest_pid = pid;
		p.source_pid = myPID;
		p.size = sizeof(T);
		p.source = (uint64_t)&data;
		p.destination = addy;

		p.opcode = OP_WRITE;

		while (p.opcode != OP_DONE)
			Sleep(1);

		p.opcode = OP_INVALID;
	}

	//��������� ����� ��� ������ �� ������ ����� �������
	template <typename T>
	T read_memory(int pid, uint64_t addy, Packet& p)
	{
		T buffer{};

		if (!pid || !addy) return buffer;

		p.dest_pid = myPID;
		p.source_pid = pid;
		p.size = sizeof(T);
		p.source = addy;
		p.destination = (uint64_t)&buffer;
		p.opcode = OP_READ;

		while (p.opcode != OP_DONE)
			Sleep(1);

		p.opcode = OP_INVALID;

		return buffer;
	}
}

//������� ��������� ����� ��� ����������
struct Color
{
	float r, g, b;
};

//���������� �����
Color green = {0.1f, 2.f, 0.1f};
Color orange = {2.f, 1.3f, 0.1f};
Color purple = {1.7f, 0.f, 2.f};


//������� ��������� ����� ���������
struct glow
{
	char pad_0000[464]; //0x0000
	Color m_clrGlow; //0x01D0   // COLOR RGB
	char pad_01DC[244]; //0x01DC
	float flt1; //0x02D0  //Highlight
	float flt2; //0x02D4  //Highlight
	float flt3; //0x02D8  //Highlight  
	float flt4; //0x02DC  //Highlight
	float flt5; //0x02E0  //Highlight
	float flt6; //0x02E4  //Highlight
	float flt7; //0x02E8  //Highlight
	char pad_02EC[16]; //0x02EC
	float m_fGlowRange; //0x02FC //Highlight_GetFarFadeDist
	char pad_0300[16]; //0x0300
	int m_iGlowContext; //0x0310 //Glow Highlight_SetCurrentContext
	char pad_0314[124]; //0x0314
	bool m_bGlowEnabled; //0x0390
};


//��������� ������
struct Player
{
	char pad_0000[332]; //0x0000
	Vector3 m_vecOrigin; //0x014C
	char pad_0158[24]; //0x0158
	int m_iShield; //0x0170
	char pad_0174[620]; //0x0174
	int m_iHealth; //0x03E0
	char pad_03E4[12]; //0x03E4
	int m_iTeam; //0x03F0
	char pad_03F4[40]; //0x03F4
	Vector3 m_vecVelocity; //0x041C
	char pad_0428[2744]; //0x0428
	uintptr_t m_pBoneArray; //0x0EE0
	char pad_0EE8[3200]; //0x0EE8
	Vector3 m_vecCameraPos; //0x1B68
	char pad_1B70[1356]; //0x1B70
	Vector3 m_vecAimpunch; //0x20C0
	char pad_20CC[188]; //0x20CC
	Vector3 m_vecViewAng; //0x2188
};

//���������� �� ����� name �������� val � ����� HKEY_LOCAL_MACHINE/SOFTWARE/WowAesdt
bool SetRegKey(DWORD val, LPCSTR name)
{
	HKEY hKey;
	LPCSTR sKeyPath = "SOFTWARE\\WowAesdt";

	int iResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sKeyPath, NULL, KEY_ALL_ACCESS, &hKey);
	iResult += RegSetValueEx(hKey, name, NULL, REG_DWORD, (const BYTE*)&val, sizeof(val));
	RegCloseKey(hKey);

	if (iResult == 0)
		return true;
	else
		return false;
}

//������ �� �������
DWORD GetRegKey(LPCSTR name)
{
	HKEY hKey;
	LPCSTR sKeyPath = "SOFTWARE\\WowAesdt";
	DWORD value;
	DWORD value_length = sizeof(DWORD);
	int iResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sKeyPath, NULL, KEY_ALL_ACCESS, &hKey);
	iResult += RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)&value, &value_length);
	RegCloseKey(hKey);
	return value;
}


//====================   AIM
//
#define M_PI 3.14159265358979323846

//
//
//�������� ���������� ������
Vector3 GetEntityBonePosition(Player DTEntity, uint32_t BoneId, int pid)
{
	Vector3 EntityHead = Vector3();

	//float rng = randomfloat(0.f, 1.f);

	EntityHead.x = PTransponder::read_memory<float>(pid, DTEntity.m_pBoneArray + 0xCC + (BoneId * 0x30), p3) + DTEntity
	                                                                                                           .m_vecOrigin
	                                                                                                           .x;
	// +rng;
	EntityHead.y = PTransponder::read_memory<float>(pid, DTEntity.m_pBoneArray + 0xDC + (BoneId * 0x30), p3) + DTEntity
	                                                                                                           .m_vecOrigin
	                                                                                                           .y;
	// +rng;
	EntityHead.z = PTransponder::read_memory<float>(pid, DTEntity.m_pBoneArray + 0xEC + (BoneId * 0x30), p3) + DTEntity
	                                                                                                           .m_vecOrigin
	                                                                                                           .z;
	// +rng;

	return EntityHead;
}
//
void SetViewAngles(uintptr_t ent, Vector3 angles, int pid)
{
	PTransponder::write_memory<Vector3>(pid, angles, ent + OFFSET_VIEWANGLES, p2);
}


namespace Process
{
	//�������� Process ID
	int get_pid(const char* processName)
	{
		//const char* processName = "r5apex.exe";

		PROCESSENTRY32 procEntry32;
		procEntry32.dwSize = sizeof(PROCESSENTRY32);

		HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcSnap == INVALID_HANDLE_VALUE)
			return -1;

		if (Process32First(hProcSnap, &procEntry32))
		{
			if (!strcmp(processName, procEntry32.szExeFile))
			{
				return procEntry32.th32ProcessID;
				CloseHandle(hProcSnap);
			}
		}

		while (Process32Next(hProcSnap, &procEntry32))
		{
			if (!strcmp(processName, procEntry32.szExeFile))
			{
				return procEntry32.th32ProcessID;
				CloseHandle(hProcSnap);
			}
		}

		CloseHandle(hProcSnap);
		return -1;
	}
}

#define OFFSET_GLOW_COLORS			0x1D0
#define OFFSET_GLOW_FADE			0x2D0
#define OFFSET_GLOW_RANGE			0x2FC //Highlight GetFarFadeDist
#define OFFSET_GLOW_CONTEXT			0x310 //Highlight SetCurrentContext
#define OFFSET_GLOW_ENABLE			0x390 //Highlight Enable


void hight_Lighty(DWORD targetpid, uint64_t entity, Color color)
{
	float maxGlow = FLT_MAX; // ������������� ����� ������������ �������� float
	int m_iGlowContext = 1; // Highlight_SetCurrentContext	
	bool m_bGlowEnabled = true; // 0x0390
	//������������� ����
	PTransponder::write_memory<Color>(targetpid, color, entity + OFFSET_GLOW_COLORS, p1);

	/*

	����������� �� ��������� glow

	float flt1; //0x02D0  //Highlight
	float flt2; //0x02D4  //Highlight
	float flt3; //0x02D8  //Highlight
	float flt4; //0x02DC  //Highlight
	float flt5; //0x02E0  //Highlight
	float flt6; //0x02E4  //Highlight
	float flt7; //0x02E8  //Highlight
	 */
	for (auto offset = OFFSET_GLOW_FADE; offset <= 0x2E8; offset += 0x4)
		PTransponder::write_memory<float>(targetpid, maxGlow, entity + offset, p1);

	//������������� ������������ ��������� ����������� GLOW
	PTransponder::write_memory<float>(targetpid, maxGlow, entity + OFFSET_GLOW_RANGE, p1);

	//�������� ���������
	PTransponder::write_memory<int>(targetpid, m_iGlowContext, entity + OFFSET_GLOW_CONTEXT, p1);

	//�������� ���������
	PTransponder::write_memory<bool>(targetpid, m_bGlowEnabled, entity + OFFSET_GLOW_ENABLE, p1);
}


namespace Math
{
	//�������� ��������� ����� ����� ������������
	float getDistance(Vector3 from, Vector3 to)
	{
		Vector3 delta;

		//�������� ������� �� ������ �� ���� ���������
		delta.x = from.x - to.x;
		delta.y = from.y - to.y;
		delta.z = from.z - to.z;

		//�������� � ��������
		delta.x *= delta.x;
		delta.y *= delta.y;
		delta.z *= delta.z;

		//��������� � ���������� ���������� ������ = ������� ���� ��������� �� ����
		return sqrt(delta.x + delta.y + delta.z); /// 39.62; // ������� �� 39.62 ��������� ��������� � �����
	}

	//����������� ����� PITCH & YAW
	Vector3 calculateAngle(const Vector3& src, const Vector3& dst)
	{
		Vector3 dsta = {dst.x, dst.y, dst.z};
		Vector3 srca = {src.x, src.y, src.z};

		Vector3 angle;
		Vector3 delta;

		delta.x = srca.x - dsta.x;
		delta.y = srca.y - dsta.y;
		delta.z = srca.z - dsta.z;

		double hyppotenuse = sqrtf((double)(delta.x * delta.x) + (delta.y * delta.y));

		angle.x = atanf(delta.z / hyppotenuse) * (180.0f / M_PI);
		angle.y = atanf(delta.y / delta.x) * (180.0f / M_PI);
		angle.z = 0.0f;

		//std::cout << "angle.x: " << angle.x << " angle.y: " << angle.y << " angle.z: " << angle.z << std::endl;

		if (delta.x >= 0.0f) angle.y += 180.0f;
		return angle;
	}

	//��������� ������������ ���� � ������������� ���������� � ����
	void normalizeAngles(Vector3& angle)
	{
		while (angle.x > 89.0f)
			angle.x -= 180.f;

		while (angle.x < -89.0f)
			angle.x += 180.f;

		while (angle.y > 180.f)
			angle.y -= 360.f;

		while (angle.y < -180.f)
			angle.y += 360.f;
	}

	//����������� Feeld of view
	float calculateFov(Vector3 ViewAngles, Vector3 LocalCamera, Vector3 targetPosition)
	{
		Vector3 angle = calculateAngle(LocalCamera, targetPosition);
		Vector3 delta;

		delta.x = angle.x - ViewAngles.x;
		delta.y = angle.y - ViewAngles.y;
		delta.z = angle.z - ViewAngles.z;

		//std::cout << "angle.x: " << angle.x << " angle.y: " << angle.y << " angle.z: " << angle.z << std::endl;
		//std::cout << "delta.x: " << delta.x << " delta.y: " << delta.y << " delta.z: " << delta.z << std::endl;

		normalizeAngles(delta);

		//std::cout << "Norm-delta.x: " << delta.x << " Norm-delta.y: " << delta.y << " Norm-delta.z: " << delta.z << std::endl;

		return sqrt(pow(delta.x, 2.0f) + pow(delta.y, 2.0f));
	}
}

namespace AIM
{
	//������� ��������� ������, � ������� ����� �������������� AIM.
	void checkAimEntity(Player LPlayer, Player Enemy, Vector3 EnemyHeadPosition, uint64_t entity)
	{
		float distance = Math::getDistance(LPlayer.m_vecOrigin, Enemy.m_vecOrigin);

		if (distance > 8000) return; // �������� �� ���������
		if (Enemy.m_iHealth < 1 || Enemy.m_iHealth > 100) return; // �������� ��� ���� ����		
		if (Enemy.m_iTeam < 0 || Enemy.m_iTeam > 50 || Enemy.m_iTeam == LPlayer.m_iTeam) return;
		//�������� ��� �� �� � ����� �������

		float FOV = Math::calculateFov(LPlayer.m_vecViewAng, LPlayer.m_vecCameraPos, EnemyHeadPosition);


		//float FOV = GetFov(LPlayer.m_vecViewAng, LPlayer.m_vecCameraPos, EnemyHeadPosition);
		//std::cout << "distance: " << distance << std::endl;
		//std::cout << "Health: " << Enemy.m_iHealth << std::endl;
		//std::cout << "Team : " << Enemy.m_iTeam << std::endl;
		//std::cout << "FOV : " << FOV << std::endl;


		//������� ���������� ����
		if (FOV < g_max_fov)
		{
			g_max_fov = FOV;
			tempAimEntyty = entity;
		}
	}
}



static DWORD WINAPI aim_thread(void* params)
{
	while (true)
	{
		Sleep(1);
		if (aimentity && (GetKeyState(VK_RBUTTON) & 0x8000))
		{
			double smooth = 10.0f;
			Player LocalPlayer = PTransponder::read_memory<Player>(targetpid, localPlayerEntity, p2);
			Player DTaimentity = PTransponder::read_memory<Player>(targetpid, aimentity, p2);
			Vector3 EnemyHeadPosition = GetEntityBonePosition(DTaimentity, 5, targetpid);
			Vector3 angle = Math::calculateAngle(LocalPlayer.m_vecCameraPos, EnemyHeadPosition);
			float fov = Math::calculateFov(LocalPlayer.m_vecViewAng, LocalPlayer.m_vecCameraPos, EnemyHeadPosition);

			//std::cout << "FOV: " << fov << " ANGLE X: " << angle.x << " ANGLE Y: " << angle.y << " ANGLE Z: " << angle.z << std::endl;
			if (fov > g_setup_fov)
			{
				continue;
			}
			Vector3 delta;
			delta.x = angle.x - LocalPlayer.m_vecViewAng.x;
			delta.y = angle.y - LocalPlayer.m_vecViewAng.y;

			Math::normalizeAngles(delta);

			delta.x /= smooth;
			delta.y /= smooth;

			Vector3 smoothedAngles;
			smoothedAngles.x = LocalPlayer.m_vecViewAng.x + delta.x;
			smoothedAngles.y = LocalPlayer.m_vecViewAng.y + delta.y;

			//Math::normalizeAngles(smoothedAngles);
			Math::normalizeAngles(angle);

			//std::cout << "smoothedAngles X:  " << smoothedAngles.x << "smoothedAngles Y:  " << smoothedAngles.y << std::endl;

			SetViewAngles(localPlayerEntity, smoothedAngles, targetpid);
		}
	}
}

int main()
{
	//������� ����� ����� ��������
	SetRegKey(0, "offset1");
	SetRegKey(0, "offset2");
	SetRegKey(0, "offset3");
	SetRegKey(0, "pid");

	uint64_t base = (uint64_t)GetModuleHandle(0); // ������� ����� aClient
	uint64_t packetAdress1 = (uint64_t)&p1;
	// ����� �� �������� �������� ���������� p - ���������� �� ������/������������
	uint64_t packetAdress2 = (uint64_t)&p2;
	// ����� �� �������� �������� ���������� p - ���������� �� ������/������������
	uint64_t packetAdress3 = (uint64_t)&p3;
	// ����� �� �������� �������� ���������� p - ���������� �� ������/������������

	myPID = GetCurrentProcessId(); // pID aClient

	DWORD tid = 0;
	CreateThread(0, 0, aim_thread, 0, 0, &tid);


	std::cout << "packet base1: " << std::hex << packetAdress1 << std::endl;
	std::cout << "exe base:    " << std::hex << base << std::endl;
	std::cout << "offset1:      " << std::hex << packetAdress1 - base << std::endl;

	//���� ID ��������
	targetpid = Process::get_pid("r5apex.exe");
	if (targetpid == -1) //���� �� ����� �����
	{
		//���� �� ����� - �������� ������
		SetRegKey(0, "offset1");
		SetRegKey(0, "offset2");
		SetRegKey(0, "offset3");
		SetRegKey(0, "pid");
		exit(0); //�������� ������
	}

	//���� �������� 0 � offset �� ��������� ���� ��������
	if (!GetRegKey("offset1"))
		SetRegKey(packetAdress1 - base, "offset1");
	//���� �������� 0 � offset �� ��������� ���� ��������
	if (!GetRegKey("offset2"))
		SetRegKey(packetAdress2 - base, "offset2");
	//���� �������� 0 � offset �� ��������� ���� ��������
	if (!GetRegKey("offset3"))
		SetRegKey(packetAdress3 - base, "offset3");
	//���� �������� 0 � pid �� ��������� ���� ��������
	if (!GetRegKey("pid"))
		SetRegKey(myPID, "pid");
	
	while (true)
	{


		//std::cout << "[v] Loaded!" << std::endl;
		//std::cout << "packet base: " << std::hex << baseofstruct << std::endl;
		//std::cout << "myPID:    " << myPID << std::endl;
		//std::cout << "targetpid:      " << targetpid << std::endl;


		//�������� ������� ������ APEX LEGEND
		uint64_t tragetBaseAdress = PTransponder::get_base(targetpid, p1);

		//������ ���� ��������
		uint64_t entitylist = tragetBaseAdress + OFFSET_ENTITYLIST;

		//������� ����� ���������
		uint64_t baseEntity = 0;

		//��������� ���������
		glow enemy;

		//������� ��������� ������
		Player LocalPlayer;

		//�������� ��������� ��� �����
		Player Enemy;

		Player DTaimentity;

		//����� ��������� ������� �� �������� ������� ���������� ������������ FOV
		g_max_fov = 9999;

		//�������� ������ �� ���������� ������
		localPlayerEntity = PTransponder::read_memory<uint64_t>(targetpid, tragetBaseAdress + OFFSET_LOCAL_ENT, p1);
		if (!localPlayerEntity) continue; //���� �� �� ����� ���������� ������, �������� �������

		//��������� ������ � ��������� ������
		LocalPlayer = PTransponder::read_memory<Player>(targetpid, localPlayerEntity, p1);


		//������� �� �� �����		
		/*std::cout << "hp: " << LocalPlayer.m_iHealth << std::endl;
		std::cout << "armor: " << LocalPlayer.m_iShield << std::endl;
		std::cout << "pos: " << LocalPlayer.m_vecOrigin.x << LocalPlayer.m_vecOrigin.y << LocalPlayer.m_vecOrigin.z << std::endl;*/


		//�������� ������� ����� ���������
		baseEntity = PTransponder::read_memory<uint64_t>(targetpid, entitylist, p1);
		if (!baseEntity) continue;

		if (baseEntity)
		{
			for (int i = 0; i <= 150; i++)
			{
				uint64_t entity = PTransponder::read_memory<uint64_t>(targetpid, entitylist + ((uint64_t)i << 5), p1);
				if (!entity) continue;
				if (entity == localPlayerEntity) continue; //���� ��� ��, ����������

				//check if valid
				Enemy = PTransponder::read_memory<Player>(targetpid, entity, p1);
				if (Enemy.m_iHealth < 1 || Enemy.m_iHealth > 100) continue;
				//if (Enemy.m_iShield > 100) continue;

				//esp				
				if (Enemy.m_iTeam == LocalPlayer.m_iTeam)
					hight_Lighty(targetpid, entity, green);
				else
					hight_Lighty(targetpid, entity, purple);


				/*
				 *todo
				 * ��� ����� �������� 2 ����, ���� ������� �� ��������� (pitch) � ���� - ����� �� ����������� yaw
				 * �������� ������� ������
				 * �������� ������� ������ ����������
				 * ���� ���� � ��������, �� ����������� ��������� � �������
				 */
				LocalPlayer = PTransponder::read_memory<Player>(targetpid, localPlayerEntity, p2);
				Vector3 EnemyHeadPosition = GetEntityBonePosition(DTaimentity, 5, targetpid);
				AIM::checkAimEntity(LocalPlayer, Enemy, EnemyHeadPosition, entity);
			}
			aimentity = tempAimEntyty;
			Sleep(1);
		}
	}
}
