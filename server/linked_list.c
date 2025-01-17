#include "linked_list.h" 
#include "user.h"
#include "thread_pool.h"
// 创建新的链表节点  
ListNode* createNode(void * val) {  
    ListNode *newNode = (ListNode*)malloc(sizeof(ListNode));  
    if (!newNode) {  
        printf("Memory allocation failed\n");  
        exit(1);  
    }  
    newNode->val = val;  
    newNode->next = NULL;  
    return newNode;  
}  
  
// 在链表末尾添加元素  
void appendNode(ListNode **head, void *val) {  
    ListNode *newNode = createNode(val);  
    if (*head == NULL) {  
        *head = newNode;  
        return;  
    }  
    ListNode *current = *head;  
    while (current->next != NULL) {  
        current = current->next;  
    }
    current->next = newNode;  
}  
  
// 删除链表中值为target的节点（假设只删除一个）  
void deleteNode(ListNode **head, void * target) {  
    if (*head == NULL) return;  
  
    if ((*head)->val == target) {  
        ListNode *temp = *head;  
        *head = (*head)->next;  
        free(temp);  
        return;  
    }  
  
    ListNode *current = *head;  
    while (current->next != NULL && current->next->val != target) {  
        current = current->next;  
    }  
  
    if (current->next != NULL) {  
        ListNode *temp = current->next;  
        current->next = current->next->next;  
        free(temp);  
    }  
}  

// 删除链表中值为peerfd的节点（假设只删除一个）  
void deleteNode2(ListNode **head, int peerfd)
{
    if (*head == NULL) return;  
    user_info_t * user = (user_info_t*)((*head)->val);
  
    if (user->sockfd == peerfd) {  
        ListNode *temp = *head;
        *head = (*head)->next;
        free(temp);
        printf("free user node.\n");
        return;
    }  
  
    ListNode *current = *head;  
    while (current->next != NULL) {
        if(((user_info_t*)current->next->val)->sockfd != peerfd) {  
            current = current->next;  
        }   
    }
  
    if (current->next != NULL) {  
        ListNode *temp = current->next;  
        current->next = current->next->next;  
        free(temp);  
        printf("free user node 2.\n");
        return;
    }  
}
  
// 打印链表(仅供调试使用)
void printList(ListNode *head) {  
    ListNode *current = head;  
    while (current != NULL) {  
        printf("%x ", current->val);  
        current = current->next;  
    }  
    printf("\n");  
}  
  
// 释放链表内存
void freeList(ListNode *head) {  
    ListNode *current = head;  
    while (current != NULL) {  
        user_info_t * user = (user_info_t *)current->val;
        train_t t;
        memset(&t,0,sizeof(train_t));
        t.type = CLIENT_FORCE_EXIT;
        send(user->sockfd,&t,sizeof(t),0);
        ListNode *temp = current;  
        current = current->next;  
        free(temp);  
    }  
}  
  
