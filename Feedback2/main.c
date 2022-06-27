#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <memory.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define NUMOFLEVEL 10 // 준비큐의 갯수는 넉넉하게 10개로 설정
#define QUEUELENGTH 99 // 큐에 들어갈수 있는 프로세스는 최대 99개로 설정

typedef struct process // 프로세스 구조체
{
	int id; // 프로세스 id
	int arrivalTime; // 도착시간
	int serviceTime; // 서비스시간

	//출력용
	int startFlag;
	int startTime; // 시작시간
	int endTime; // 종료시간
	int turnAroundTime; // 반환시간
	float normaltATime; // 정규화된 반환시간

	//계산용
	int accServiceTime; // RR에서 프로세스가 몇시간 동안 실행되었는지 저장

}Process;

// input 텍스트 파일 읽어오기
int DataSet(Process queue[][QUEUELENGTH], Process* processes, int* numOfData)
{
	char line[32] = { 0, };
	FILE* in = fopen("input.txt", "r");
	if (in == NULL)
		return -1;

	int id = -1, arrivalTime = -1, serviceTime = -1;
	char tmp = '\0'; // 데이터 읽어올 때 콤마 받기 위한 변수

	int idxOfProcesses = 0;

	//input.txt에서 데이터를 읽어와 Process배열에 데이터를 순차적으로 저장
	while (1) {
		fscanf(in, "%d %c %d %c %d", &id, &tmp, &arrivalTime, &tmp, &serviceTime);
		if (feof(in) || id == 0)
			break;

		processes[idxOfProcesses].id = id;
		processes[idxOfProcesses].arrivalTime = arrivalTime;
		processes[idxOfProcesses].serviceTime = serviceTime;
		idxOfProcesses++;
	}
	*numOfData = idxOfProcesses; // 읽어온 프로세스의 갯수 저장
	printf("----- 입력된 프로세스 개수: %d -----\n", *numOfData);

	int idxOfQueue = 0;

	//순차적으로 저장한 Process배열내에 데이터 중 도착시간이 0 인 프로세스들은 Queue 0 단계에 집어넣는다.
	for (int i = 0; i < idxOfProcesses; ++i)
	{
		if (processes[i].arrivalTime == 0)
			queue[0][idxOfQueue++] = processes[i];
	}

	fclose(in);

	return idxOfQueue; // 큐에 들어온 프로세스 갯수 반환
}

//큐에 프로세스 삽입하는 함수
int InsertQueue(Process queue[QUEUELENGTH], Process process)
{
	for (int i = 0; i < QUEUELENGTH; ++i)
	{
		if (queue[i].id == 0)	// 데이터를 0으로 초기화하고 시작했기 때문에, id가 0이라면 데이터가 없는 곳
		{
			queue[i] = process;	// 해당 데이터를 삽입
			return TRUE;
		}
	}
	return FALSE;
}

// 프로세스 종료시 정보 출력
void PrintProcess(Process process, int* numOfData)
{
	printf("<<<프로세스 종료>>> ----- 남은 프로세스 개수: %d -----\n", *numOfData);
	printf("PID: %d\t도착시간: %d\t서비스시간: %d\t종료시간: %d\t반환시간: %d\t정규화된 반환시간: %.5f\n\n"
		, process.id, process.arrivalTime, process.serviceTime, process.endTime, process.turnAroundTime, process.normaltATime);
}

//시간에 따라 도착한 프로세스들을 준비큐의 0단계에 삽입
void ProcessArrive(Process queue[][QUEUELENGTH], Process processes[QUEUELENGTH], int time, int* num)
{
	for (int i = 0; i < QUEUELENGTH; ++i)
	{
		if (processes[i].arrivalTime == time) // 도착 시간과 현재 시간이 같으면 삽입
		{
			int result = InsertQueue(queue[0], processes[i]);

			if (result == TRUE)
				(*num)++; // 큐에 들어있는 프로세스 개수 1 증가
		}
	}
}

//프로세스가 끝나면 출력해야 하는 정보 계산
void ProcessEnd(Process process, int curTime, int* numOfData)
{
	process.endTime = curTime; // 종료시간
	process.turnAroundTime = curTime - process.arrivalTime; // 반환시간
	process.normaltATime = (float)process.turnAroundTime / process.serviceTime; // 정규화된 반환시간

	PrintProcess(process, numOfData); // 프로세스가 종료될때마다 6가지 내용 출력 (id, 도착시간, 서비스시간 등등)
}

// 프로세스 종료시 해당 프로세스 삭제
void DeleteQueue(Process queue[][QUEUELENGTH], int level, int idx)
{
	memset(&queue[level][idx], 0, sizeof(queue[level][idx]));

	for (int i = idx; i < QUEUELENGTH - 1; ++i)
		queue[level][i] = queue[level][i + 1];
}

/****************** 피드백 큐 (q=2^i) 시작 ******************/

void Start(Process queue[][QUEUELENGTH], Process processes[QUEUELENGTH], int* numOfDataInQueue, int* numOfData)
{
	int time = 0;
	int skipFlag = FALSE; // 프로세스가 끝났는지 확인
	while (1)
	{
		if (*numOfData == 0) // 남은 프로세스의 갯수가 0 이면 종료
			return;
		skipFlag = FALSE;

		for (int level = 0; level < NUMOFLEVEL; ++level) // 준비큐의 갯수 만큼 반복
		{
			for (int idx = 0; idx < QUEUELENGTH; ++idx) // 큐에 저장될수 있는 프로세스의 갯수만큼 반복
			{
				if (queue[level][idx].id != 0) // 프로세스의 id가 0이 아니라면
				{
					for (int accTime = 0; accTime < pow(2, level); ++accTime) // RR의 시간 할당량은 2^i 만큼 증가
					{
						if (queue[level][idx].startFlag == FALSE)
						{
							queue[level][idx].startFlag = TRUE;
							queue[level][idx].startTime = time; // 프로세스 시작시간 저장
						}
						queue[level][idx].accServiceTime++; // 프로세스의 실제 실행시간 1씩 증가

						if (queue[level][idx].accServiceTime == queue[level][idx].serviceTime)	//프로세스 서비스 다했을 경우, 종료시간 등 데이터 저장
						{
							ProcessEnd(queue[level][idx], time + 1, numOfData); //프로세스가 끝난 후 처리(정보 계산, 출력 )
							skipFlag = TRUE; // 프로세스가 끝남
							time++;
							ProcessArrive(queue, processes, time, numOfDataInQueue); // 현재 시간에 들어오는 프로세스 검사 및 삽입
							(*numOfDataInQueue)--; // 큐에 있는 프로세스 갯수 하나 감소
							DeleteQueue(queue, level, idx); // 종료된 프로세스 제거
							(*numOfData)--; // 남은 프로세스 갯수 1 감소
							break;
						}
						time++;
						ProcessArrive(queue, processes, time, numOfDataInQueue); // 현재 시간에 들어오는 프로세스 검사 및 삽입
					}
					if (skipFlag == FALSE) // 준비큐 단계의 시간할당량 동안 끝내지 못했을 경우
					{
						if (*numOfDataInQueue > 1) // 큐에 있는 프로세스가 하나 이상일 경우  
						{
							if (level != 9) // 마지막 준비큐 단계가 아닐경우엔 다음 준비큐로 삽입, 현재 큐에서 삭제  
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

// 메인함수
int main(void)
{
	Process processes[QUEUELENGTH] = { 0, }; // 99개 프로세스 구조체 배열 생성
	Process queue[NUMOFLEVEL][QUEUELENGTH] = { 0, }; // 준비큐는 넉넉하게 10개로 설정

	int numOfData = -1;
	int numOfDataInQueue = DataSet(queue, processes, &numOfData); // 데이터 읽어온 후 0번 큐에 들어온 프로세스 갯수 저장
	Start(queue, processes, &numOfDataInQueue, &numOfData); // 피드백 큐 시작
	return 0;
}