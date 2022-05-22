#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// 客户
typedef struct Customer {
	int index;
	int ArrivalTime;
	int DuraTime;
	int Amount;
}Customer;

// 队列节点
typedef struct Qnode {
	Customer* cust;
	Qnode* next;
}Qnode;

// 链队
typedef struct LinkQueue {
	Qnode* front, * rear;
}LinkQueue;

// 事件节点
typedef struct Event {
	int index;
	int OccurTime;
	int Type;
	int Amount;
	int Status;
	Event* next;
}Event;

// 事件表
typedef struct EventList {
	Event* head, * tail;
	int len;
}EventList;


// 全局变量
int CurTime = 0;				// 当前时间
int CloseTime;					// 关张时间
long int BankMoney;				// 银行余额
int TotalCustomer = 0;			// 共服务人数
int TotalArrivedCustomer = 0;	// 共到达人数
int ServiceTime = 0;			// 总服务时长
int TotalStayTime = 0;			// 客户逗留总时长
int AmountMax;					// 存取款上下界
int AmountMin;
int CustomerGapMax;				// 客户到达间隔上下界
int CustomerGapMin;
int custindex = 0;				// 客户序号
int SortDepth = 0;				// 循环变量
EventList ev;
LinkQueue* a, * b;				// 两个队列
Qnode* apos;					// 两个队列的头指针
Qnode* bpos;

// 初始化两个队列
void InitQueue() {
	a = (LinkQueue*)malloc(sizeof(LinkQueue));
	a->front = a->rear = NULL;
	b = (LinkQueue*)malloc(sizeof(LinkQueue));
	b->front = b->rear = NULL;
}

// 初始化事件表
void InitEventList() {
	ev.head = NULL;
	ev.tail = NULL;
	ev.len = 0;
}

// 开张前初始化
void InitEverything() {
	InitQueue();
	InitEventList();
}

// 将客户排入队列
void QAppendNode(Qnode* node, LinkQueue* m) {
	Qnode* pointer = m->front;
	Qnode* temp;
	while (1) {
		if (!(m->front)) {
			m->front = node;
			break;
		}
		else {
			temp = pointer;
			pointer = pointer->next;
			if (!pointer) {
				temp->next = node;
				pointer = node;
				break;
			}
		}
	}
	m->rear = node->next;
}

// 初始化一个客人
void InitCustomer(Customer* customer, int time) {
	custindex++;
	customer->index = custindex;
	customer->ArrivalTime = time;
	customer->DuraTime = rand() % 30 + 1;
	customer->Amount = rand() % (AmountMax - AmountMin) + AmountMin;
}

// 将事件排入事件链表
void EvListAppend(EventList* eventlist, Event* event) {
	Event* pointer = eventlist->head;
	Event* temp;
	while (1) {
		if (!eventlist->head) {
			eventlist->head = event;
			break;
		}
		else {
			temp = pointer;
			pointer = pointer->next;
			if (!pointer) {
				temp->next = event;
				pointer = event;
				break;
			}
		}
	}
	eventlist->tail = event->next;
	eventlist->len++;
}

// 生成事件 type 1到达 2离开 // status 0失败 1成功
void GenerateEvent(Customer* cust, int type) {
	Event* event = (Event*)malloc(sizeof(Event));
	event->Amount = cust->Amount;
	event->index = cust->index;
	if (type == 1) {
		event->OccurTime = cust->ArrivalTime;
		event->Status = 1;
	}
	else if (type == 2) {
		if (CurTime < cust->ArrivalTime)
			CurTime = cust->ArrivalTime;
		if (CurTime + cust->DuraTime > CloseTime) {
			event->OccurTime = CloseTime;
			ServiceTime += (CloseTime - CurTime);
			TotalStayTime += (CloseTime - CurTime);
			CurTime = CloseTime;
			event->Status = 0;
		}
		else {
			event->OccurTime = CurTime + cust->DuraTime;
			CurTime += cust->DuraTime;
			TotalCustomer++;
			ServiceTime += cust->DuraTime;
			TotalStayTime += (CurTime - cust->ArrivalTime);
			event->Status = 1;
		}
	}
	event->Type = type;
	event->next = NULL;
	EvListAppend(&ev, event);
}

// 生成客户 加入A队列 生成到达事件
void GenerateCustomer(int time) {
	Customer* customer = (Customer*)malloc(sizeof(Customer));
	InitCustomer(customer, time);
	Qnode* node = (Qnode*)malloc(sizeof(Qnode));
	node->cust = customer;
	node->next = NULL;
	QAppendNode(node, a);
	GenerateEvent(customer, 1);
	TotalArrivedCustomer++;
}

// 银行资金变动
void BankInOut(int amount) {
	BankMoney += amount;
}

// 离队
void DeQueue(LinkQueue* qu, Qnode* node) {
	Qnode* pointer = qu->front;
	Qnode* temp = pointer;
	if (qu->front == node) {
		qu->front = qu->front->next;
	}
	else {
		while (1) {
			if (pointer == node) {
				temp->next = node->next;
				node->next = NULL;
				node = NULL;
				break;
			}
			temp = pointer;
			pointer = pointer->next;
		}
	}
}

// 将暂时不能处理的客户移动到B队列
void MoveA2B(Qnode* node) {
	DeQueue(a, node);

	Qnode* pointer = b->front;
	Customer* customer = (Customer*)malloc(sizeof(Customer));
	Qnode* temp = (Qnode*)malloc(sizeof(Qnode));
	temp->cust = customer;
	temp->next = NULL;
	temp->cust->Amount = node->cust->Amount;
	temp->cust->ArrivalTime = node->cust->ArrivalTime;
	temp->cust->DuraTime = node->cust->DuraTime;
	temp->cust->index = node->cust->index;

	QAppendNode(temp, b);
}

// 检查B队列是否有可以取款的
void CheckQB(int before) {
	bpos = b->front;
	while (b->front != b->rear && bpos != b->rear && BankMoney > before && CurTime < CloseTime) {
		if (BankMoney + bpos->cust->Amount >= 0) {
			BankInOut(bpos->cust->Amount);
			GenerateEvent(bpos->cust, 2);
			DeQueue(b, bpos);
		}
		bpos = bpos->next;
	}
}

// 开始服务
void Service() {
	apos = a->front;
	Qnode* temp;
	CurTime += apos->cust->ArrivalTime;
	while (CurTime < CloseTime && apos != nullptr) {
		if (apos->cust->Amount >= 0) {
			BankInOut(apos->cust->Amount);
			GenerateEvent(apos->cust, 2);
			DeQueue(a, apos);
			CheckQB(BankMoney - apos->cust->Amount);
		}
		else if (apos->cust->Amount < 0) {
			if (BankMoney + apos->cust->Amount >= 0) {
				BankInOut(apos->cust->Amount);
				GenerateEvent(apos->cust, 2);
				DeQueue(a, apos);
			}
			else if (BankMoney + apos->cust->Amount < 0) {
				temp = apos;
				apos = apos->next;
				MoveA2B(temp);
				continue;
			}
		}
		apos = apos->next;
	}
}

// 将未完成服务的清除出队
void RestExitBank() {
	Qnode* p = a->front;
	Qnode* q = b->front;
	for (int i = 1; i <= TotalArrivedCustomer && p != nullptr; i++) {
		if (p != nullptr) {
			Event* event = (Event*)malloc(sizeof(Event));
			event->Amount = p->cust->Amount;
			event->index = p->cust->index;
			event->OccurTime = CloseTime;
			event->Status = 0;

			ServiceTime += (CloseTime - CurTime);
			TotalStayTime += (CloseTime - CurTime);
			CurTime = CloseTime;

			event->Type = 2;
			event->next = NULL;
			EvListAppend(&ev, event);
		}
		p = p->next;
	}
	for (int i = 1; i <= TotalArrivedCustomer && q != nullptr; i++) {
		if (q != nullptr) {
			Event* event = (Event*)malloc(sizeof(Event));
			event->Amount = q->cust->Amount;
			event->index = q->cust->index;
			event->OccurTime = CloseTime;
			event->Status = 0;

			ServiceTime += (CloseTime - CurTime);
			TotalStayTime += (CloseTime - CurTime);
			CurTime = CloseTime;

			event->Type = 2;
			event->next = NULL;
			EvListAppend(&ev, event);
		}
		q = q->next;
	}
}

// 事件表按时间排序
void SortEventList() {
	Event temp;
	Event* ptr1, * ptrtemp;
	ptr1 = ev.head;
	ptrtemp = ptr1;
	int i = 0;
	while (i <= SortDepth) {
		SortDepth = 0;
		while (ptr1->next) {
			if (ptr1->next->OccurTime < ptr1->OccurTime) {
				temp.Amount = ptr1->Amount;
				temp.index = ptr1->index;
				temp.OccurTime = ptr1->OccurTime;
				temp.Type = ptr1->Type;

				ptr1->Amount = ptr1->next->Amount;
				ptr1->index = ptr1->next->index;
				ptr1->OccurTime = ptr1->next->OccurTime;
				ptr1->Type = ptr1->next->Type;

				ptr1->next->Amount = temp.Amount;
				ptr1->next->index = temp.index;
				ptr1->next->OccurTime = temp.OccurTime;
				ptr1->next->Type = temp.Type;
			}
			ptr1 = ptr1->next;
			SortDepth++;
		}
		i++;
		ptr1 = ptrtemp;
	}
}

// 打印事件表
void PrintEvent() {
	Event* pointer = ev.head;
	while (pointer) {
		printf("\t%d\t\t", pointer->index);
		if (pointer->Type == 1)
			printf("到达\t\t");
		else if (pointer->Type == 2)
			printf("离开\t\t");
		printf("%d\t\t", pointer->OccurTime);
		printf("%d", pointer->Amount);
		if (pointer->Status == 0) {
			printf("\t（未服务）");
		}
		printf("\n");
		pointer = pointer->next;
	}
}

// 开始银行业务模拟
void BankSimulation() {

	InitEverything();

	// 生成所有客户
	for (int i = 0; i < CloseTime; i++) {
		if (i != 0) {
			GenerateCustomer(i);
		}
		int gap = rand() % (CustomerGapMax - CustomerGapMin) + CustomerGapMin;
		i += gap;
	}

	Service();
	RestExitBank();

	SortEventList();

	printf("\n\t序号\t\t事件\t\t时间\t\t数额\n\n");
	PrintEvent();


	float AverageTime = (float)TotalStayTime / TotalArrivedCustomer;

	printf("\n共到达： %d 人\n", TotalArrivedCustomer);
	printf("共接待： %d 人\n", TotalCustomer);
	printf("总计接待时长： %d 分钟\n", ServiceTime);
	printf("平均逗留时长： %.1f 分钟\n", AverageTime);
	printf("今日银行结余： %d 元\n\n", BankMoney);
}

void main(void) {
	printf("输入银行营业时长(min)\n");
	scanf_s("%d", &CloseTime);
	printf("输入银行初始存款(yuan)\n");
	scanf_s("%d", &BankMoney);
	printf("输入单次存款最大值(正数yuan)\n");
	scanf_s("%d", &AmountMax);
	printf("输入单次取款最大值(负数yuan)\n");
	scanf_s("%d", &AmountMin);
	printf("输入两个客户时间最大间隔(min)\n");
	scanf_s("%d", &CustomerGapMax);
	printf("输入两个客户时间最小间隔(min)\n");
	scanf_s("%d", &CustomerGapMin);

	srand((unsigned)time(NULL));

	BankSimulation();

	system("pause");
}