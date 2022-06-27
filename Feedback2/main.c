#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <memory.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define NUMOFLEVEL 10 // �غ�ť�� ������ �˳��ϰ� 10���� ����
#define QUEUELENGTH 99 // ť�� ���� �ִ� ���μ����� �ִ� 99���� ����

typedef struct process // ���μ��� ����ü
{
	int id; // ���μ��� id
	int arrivalTime; // �����ð�
	int serviceTime; // ���񽺽ð�

	//��¿�
	int startFlag;
	int startTime; // ���۽ð�
	int endTime; // ����ð�
	int turnAroundTime; // ��ȯ�ð�
	float normaltATime; // ����ȭ�� ��ȯ�ð�

	//����
	int accServiceTime; // RR���� ���μ����� ��ð� ���� ����Ǿ����� ����

}Process;

// input �ؽ�Ʈ ���� �о����
int DataSet(Process queue[][QUEUELENGTH], Process* processes, int* numOfData)
{
	char line[32] = { 0, };
	FILE* in = fopen("input.txt", "r");
	if (in == NULL)
		return -1;

	int id = -1, arrivalTime = -1, serviceTime = -1;
	char tmp = '\0'; // ������ �о�� �� �޸� �ޱ� ���� ����

	int idxOfProcesses = 0;

	//input.txt���� �����͸� �о�� Process�迭�� �����͸� ���������� ����
	while (1) {
		fscanf(in, "%d %c %d %c %d", &id, &tmp, &arrivalTime, &tmp, &serviceTime);
		if (feof(in) || id == 0)
			break;

		processes[idxOfProcesses].id = id;
		processes[idxOfProcesses].arrivalTime = arrivalTime;
		processes[idxOfProcesses].serviceTime = serviceTime;
		idxOfProcesses++;
	}
	*numOfData = idxOfProcesses; // �о�� ���μ����� ���� ����
	printf("----- �Էµ� ���μ��� ����: %d -----\n", *numOfData);

	int idxOfQueue = 0;

	//���������� ������ Process�迭���� ������ �� �����ð��� 0 �� ���μ������� Queue 0 �ܰ迡 ����ִ´�.
	for (int i = 0; i < idxOfProcesses; ++i)
	{
		if (processes[i].arrivalTime == 0)
			queue[0][idxOfQueue++] = processes[i];
	}

	fclose(in);

	return idxOfQueue; // ť�� ���� ���μ��� ���� ��ȯ
}

//ť�� ���μ��� �����ϴ� �Լ�
int InsertQueue(Process queue[QUEUELENGTH], Process process)
{
	for (int i = 0; i < QUEUELENGTH; ++i)
	{
		if (queue[i].id == 0)	// �����͸� 0���� �ʱ�ȭ�ϰ� �����߱� ������, id�� 0�̶�� �����Ͱ� ���� ��
		{
			queue[i] = process;	// �ش� �����͸� ����
			return TRUE;
		}
	}
	return FALSE;
}

// ���μ��� ����� ���� ���
void PrintProcess(Process process, int* numOfData)
{
	printf("<<<���μ��� ����>>> ----- ���� ���μ��� ����: %d -----\n", *numOfData);
	printf("PID: %d\t�����ð�: %d\t���񽺽ð�: %d\t����ð�: %d\t��ȯ�ð�: %d\t����ȭ�� ��ȯ�ð�: %.5f\n\n"
		, process.id, process.arrivalTime, process.serviceTime, process.endTime, process.turnAroundTime, process.normaltATime);
}

//�ð��� ���� ������ ���μ������� �غ�ť�� 0�ܰ迡 ����
void ProcessArrive(Process queue[][QUEUELENGTH], Process processes[QUEUELENGTH], int time, int* num)
{
	for (int i = 0; i < QUEUELENGTH; ++i)
	{
		if (processes[i].arrivalTime == time) // ���� �ð��� ���� �ð��� ������ ����
		{
			int result = InsertQueue(queue[0], processes[i]);

			if (result == TRUE)
				(*num)++; // ť�� ����ִ� ���μ��� ���� 1 ����
		}
	}
}

//���μ����� ������ ����ؾ� �ϴ� ���� ���
void ProcessEnd(Process process, int curTime, int* numOfData)
{
	process.endTime = curTime; // ����ð�
	process.turnAroundTime = curTime - process.arrivalTime; // ��ȯ�ð�
	process.normaltATime = (float)process.turnAroundTime / process.serviceTime; // ����ȭ�� ��ȯ�ð�

	PrintProcess(process, numOfData); // ���μ����� ����ɶ����� 6���� ���� ��� (id, �����ð�, ���񽺽ð� ���)
}

// ���μ��� ����� �ش� ���μ��� ����
void DeleteQueue(Process queue[][QUEUELENGTH], int level, int idx)
{
	memset(&queue[level][idx], 0, sizeof(queue[level][idx]));

	for (int i = idx; i < QUEUELENGTH - 1; ++i)
		queue[level][i] = queue[level][i + 1];
}

/****************** �ǵ�� ť (q=2^i) ���� ******************/

void Start(Process queue[][QUEUELENGTH], Process processes[QUEUELENGTH], int* numOfDataInQueue, int* numOfData)
{
	int time = 0;
	int skipFlag = FALSE; // ���μ����� �������� Ȯ��
	while (1)
	{
		if (*numOfData == 0) // ���� ���μ����� ������ 0 �̸� ����
			return;
		skipFlag = FALSE;

		for (int level = 0; level < NUMOFLEVEL; ++level) // �غ�ť�� ���� ��ŭ �ݺ�
		{
			for (int idx = 0; idx < QUEUELENGTH; ++idx) // ť�� ����ɼ� �ִ� ���μ����� ������ŭ �ݺ�
			{
				if (queue[level][idx].id != 0) // ���μ����� id�� 0�� �ƴ϶��
				{
					for (int accTime = 0; accTime < pow(2, level); ++accTime) // RR�� �ð� �Ҵ緮�� 2^i ��ŭ ����
					{
						if (queue[level][idx].startFlag == FALSE)
						{
							queue[level][idx].startFlag = TRUE;
							queue[level][idx].startTime = time; // ���μ��� ���۽ð� ����
						}
						queue[level][idx].accServiceTime++; // ���μ����� ���� ����ð� 1�� ����

						if (queue[level][idx].accServiceTime == queue[level][idx].serviceTime)	//���μ��� ���� ������ ���, ����ð� �� ������ ����
						{
							ProcessEnd(queue[level][idx], time + 1, numOfData); //���μ����� ���� �� ó��(���� ���, ��� )
							skipFlag = TRUE; // ���μ����� ����
							time++;
							ProcessArrive(queue, processes, time, numOfDataInQueue); // ���� �ð��� ������ ���μ��� �˻� �� ����
							(*numOfDataInQueue)--; // ť�� �ִ� ���μ��� ���� �ϳ� ����
							DeleteQueue(queue, level, idx); // ����� ���μ��� ����
							(*numOfData)--; // ���� ���μ��� ���� 1 ����
							break;
						}
						time++;
						ProcessArrive(queue, processes, time, numOfDataInQueue); // ���� �ð��� ������ ���μ��� �˻� �� ����
					}
					if (skipFlag == FALSE) // �غ�ť �ܰ��� �ð��Ҵ緮 ���� ������ ������ ���
					{
						if (*numOfDataInQueue > 1) // ť�� �ִ� ���μ����� �ϳ� �̻��� ���  
						{
							if (level != 9) // ������ �غ�ť �ܰ谡 �ƴҰ�쿣 ���� �غ�ť�� ����, ���� ť���� ����  
							{
								InsertQueue(queue[level + 1], queue[level][idx]);
								DeleteQueue(queue, level, idx);
							}
						}
						skipFlag = TRUE;
					}
				}
				if (skipFlag)
					break;
			}
			if (skipFlag)
				break;
		}
	}
}

// �����Լ�
int main(void)
{
	Process processes[QUEUELENGTH] = { 0, }; // 99�� ���μ��� ����ü �迭 ����
	Process queue[NUMOFLEVEL][QUEUELENGTH] = { 0, }; // �غ�ť�� �˳��ϰ� 10���� ����

	int numOfData = -1;
	int numOfDataInQueue = DataSet(queue, processes, &numOfData); // ������ �о�� �� 0�� ť�� ���� ���μ��� ���� ����
	Start(queue, processes, &numOfDataInQueue, &numOfData); // �ǵ�� ť ����
	return 0;
}