// MBR.cpp - 分析MBR主引导结构
// MBR-512Bytes（引导代码446Bytes,分区表64Bytes,结束符2Bytes）

#include <windows.h>
#include <winioctl.h> // DDK 驱动开发
#include <stdio.h>

// 引导程序大小
#define BOOTRECORDSIZE 440

typedef struct _BOOTRECORD
{
	u_char BootRecord[BOOTRECORDSIZE]; 
}BOOTRECORD, *PBOOTRECORD;

// 分区表大小
#define DPTSIZE 64 

typedef struct _DPT
{
	u_char Dpt[DPTSIZE]; 
}DPT, *PDPT;

/* MBR 主引导记录结构定义 */
typedef struct _MBR
{
	BOOTRECORD BootRecord;	// 引导程序
	u_char	   ulSigned[4]; // Windows 磁盘签名
	u_char     sReserve[2]; // 保留
	DPT	       Dpt;		    // 分区表
	u_char     EndSign[2];	// 结束标志
}MBR, *PMBR;

// 主分区个数
#define DPTNUMBER 4 

// 分区的结构 16 Bytes
typedef struct _DP
{
	u_char BootSign;		   // 引导标志 0x80 为激活, 0x00 为非激活
	u_char StartHsc[3];		   // 起始磁头号(1字节), 扇区号(2字节低6位), 柱面号(2字节高2位+3字节)
	u_char PartitionType;	   // 分区类型 0x05, 0x0F 为扩展分区 0x06 FAT16分区 0x0B(FAT32分区) 0x07NTFS分区
	u_char EndHsc[3];		   // 结束磁头号(1字节), 扇区号(2字节低6位), 柱面号(2字节高2位+3字节)
	u_long SectorsPreceding;   // 本分区之前使用的扇区数 
	u_long SectorsInPartition; // 分区的总扇区数
}DP, *PDP;
// 为 0xF 或 0x5 分区类型时，表示分区为扩展分区，被分成一个一个的逻辑分区

VOID ShowMbr(HANDLE hDevice, PMBR pMbr)
{
	DWORD dwRead = 0;
	ReadFile(hDevice, (LPVOID)pMbr, sizeof(MBR), &dwRead, NULL);
	printf("主引导记录: \r\n");
	for (int i = 0; i < 512; i++)
	{
		printf("%02X ", ((BYTE*)pMbr)[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}
}

VOID ParseMbr(MBR Mbr)
{
	printf("主引导程序: \r\n");
	int i;

	for (i = 0; i < BOOTRECORDSIZE; i++)
	{
		printf("%02X ", Mbr.BootRecord.BootRecord[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}
	printf("\r\n");
	printf("磁盘签名: \r\n");
	for (i = 0; i < 4; i++)
	{
		printf("%02X ", Mbr.ulSigned[i]);
	}
	printf("\r\n");
	printf("解析分区表: \r\n");
	for (i = 0; i < DPTSIZE; i++)
	{
		printf("%02X ", Mbr.Dpt.Dpt[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\r\n");
		}
	}
	printf("\r\n");
	PDP pDp = (PDP)(&Mbr.Dpt.Dpt);
	for (i = 0; i < DPTNUMBER; i++)
	{
		printf("引导标志: %02X ", pDp[i].BootSign);
		printf("分区类型: %02X\r\n", pDp[i].PartitionType);
		printf("本分区之前扇区数: %02X ", pDp[i].SectorsPreceding);
		printf("本分区的总扇区数: %02X\r\n", pDp[i].SectorsInPartition);
		printf("该分区的大小: %fGB\r\n", (double)pDp[i].SectorsInPartition * 512 / 1024
			/ 1024 / 1024);
		printf("\r\n\r\n");
	}
	printf("结束标志: \r\n");
	for (i = 0; i < 2; i++)
	{
		printf("%02X ", Mbr.EndSign[i]);
	}
	printf("\r\n");
}

int main()
{
    // 打开物理硬盘设备
	HANDLE hDevice = CreateFile(L"\\\\.\\PhysicalDrive0",
								GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								0,
								NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile Error %s\r\n", GetLastError());
		return -1;
	}
	MBR Mbr = { 0 };
	ShowMbr(hDevice, &Mbr);
	ParseMbr(Mbr);
	CloseHandle(hDevice);
	system("pause");
	return 0;
}


