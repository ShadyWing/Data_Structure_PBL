#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// �ͻ�
typedef struct Customer {
	int index;
	int ArrivalTime;
	int DuraTime;
	int Amount;
}Customer;

// ���нڵ�
typedef struct Qnode {
	Customer* cust;
	Qnode* next;
}Qnode;

// ����
typedef struct LinkQueue {
	Qnode* front, * rear;
}LinkQueue;

// �¼��ڵ�
typedef struct Event {
	int index;
	int OccurTime;
	int Type;
	int Amount;
	int Status;
	Event* next;
}Event;

// �¼���
typedef struct EventList {
	Event* head, * tail;
	int len;
}EventList;


// ȫ�ֱ���
int CurTime = 0;				// ��ǰʱ��
int CloseTime;					// ����ʱ��
long int BankMoney;				// �������
int TotalCustomer = 0;			// ����������
int TotalArrivedCustomer = 0;	// ����������
int ServiceTime = 0;			// �ܷ���ʱ��
int TotalStayTime = 0;			// �ͻ�������ʱ��
int AmountMax;					// ��ȡ�����½�
int AmountMin;
int CustomerGapMax;				// �ͻ����������½�
int CustomerGapMin;
int custindex = 0;				// �ͻ����
int SortDepth = 0;				// ѭ������
EventList ev;
LinkQueue* a, * b;				// ��������
Qnode* apos;					// �������е�ͷָ��
Qnode* bpos;

// ��ʼ����������
void InitQueue() {
	a = (LinkQueue*)malloc(sizeof(LinkQueue));
	a->front = a->rear = NULL;
	b = (LinkQueue*)malloc(sizeof(LinkQueue));
	b->front = b->rear = NULL;
}

// ��ʼ���¼���
void InitEventList() {
	ev.head = NULL;
	ev.tail = NULL;
	ev.len = 0;
}

// ����ǰ��ʼ��
void InitEverything() {
	InitQueue();
	InitEventList();
}

// ���ͻ��������
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

// ��ʼ��һ������
void InitCustomer(Customer* customer, int time) {
	custindex++;
	customer->index = custindex;
	customer->ArrivalTime = time;
	customer->DuraTime = rand() % 30 + 1;
	customer->Amount = rand() % (AmountMax - AmountMin) + AmountMin;
}

// ���¼������¼�����
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

// �����¼� type 1���� 2�뿪 // status 0ʧ�� 1�ɹ�
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

// ���ɿͻ� ����A���� ���ɵ����¼�
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

// �����ʽ�䶯
void BankInOut(int amount) {
	BankMoney += amount;
}

// ���
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

// ����ʱ���ܴ���Ŀͻ��ƶ���B����
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

// ���B�����Ƿ��п���ȡ���
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

// ��ʼ����
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

// ��δ��ɷ�����������
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

// �¼���ʱ������
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

// ��ӡ�¼���
void PrintEvent() {
	Event* pointer = ev.head;
	while (pointer) {
		printf("\t%d\t\t", pointer->index);
		if (pointer->Type == 1)
			printf("����\t\t");
		else if (pointer->Type == 2)
			printf("�뿪\t\t");
		printf("%d\t\t", pointer->OccurTime);
		printf("%d", pointer->Amount);
		if (pointer->Status == 0) {
			printf("\t��δ����");
		}
		printf("\n");
		pointer = pointer->next;
	}
}

// ��ʼ����ҵ��ģ��
void BankSimulation() {

	InitEverything();

	// �������пͻ�
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

	printf("\n\t���\t\t�¼�\t\tʱ��\t\t����\n\n");
	PrintEvent();


	float AverageTime = (float)TotalStayTime / TotalArrivedCustomer;

	printf("\n����� %d ��\n", TotalArrivedCustomer);
	printf("���Ӵ��� %d ��\n", TotalCustomer);
	printf("�ܼƽӴ�ʱ���� %d ����\n", ServiceTime);
	printf("ƽ������ʱ���� %.1f ����\n", AverageTime);
	printf("�������н��ࣺ %d Ԫ\n\n", BankMoney);
}

void main(void) {
	printf("��������Ӫҵʱ��(min)\n");
	scanf_s("%d", &CloseTime);
	printf("�������г�ʼ���(yuan)\n");
	scanf_s("%d", &BankMoney);
	printf("���뵥�δ�����ֵ(����yuan)\n");
	scanf_s("%d", &AmountMax);
	printf("���뵥��ȡ�����ֵ(����yuan)\n");
	scanf_s("%d", &AmountMin);
	printf("���������ͻ�ʱ�������(min)\n");
	scanf_s("%d", &CustomerGapMax);
	printf("���������ͻ�ʱ����С���(min)\n");
	scanf_s("%d", &CustomerGapMin);

	srand((unsigned)time(NULL));

	BankSimulation();

	system("pause");
}