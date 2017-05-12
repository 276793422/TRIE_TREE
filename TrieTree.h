//	Author: Zoo
//	QQ:276793422
//
//		ģ��˵��
//	��ǰģ����һ��֧�ֿ�ƽ̨��ȫ��̬�����ǰ׺����
//	����ǰһ������Ҫ�õ�һ��ǰ׺����
//	��������������һ���������Ҿ�û���ҵ�ȫ��̬�����ڴ��ǰ׺����
//	���ϵĸ���ǰ׺���������б�Ԫ�ظ���ȷ���ģ�
//	��������ǰ׺����Ŀ������������ļ�·����
//	�ļ�·�����ַ����ǰ��������ַ��ģ�
//	��������������б�Ԫ�ظ���ȷ���Ļ���
//	��ô�Ϳ�����Ҫÿ�����ڵ㶼��Ҫ0xFFFF��ָ��Ԫ�أ�
//	���������������ͷ��ˣ��ڴ��˷ѹ��࣬x86������һ���ڵ��Ҫ�õ�256K�ڴ棬x64�·�����
//	�����ҵ��������ҵ�һ��ȫ��̬�ڴ������ǰ׺����
//	���ǣ������˸���ƽ̨������Դ���룬��û���ҵ���
//	Ҳ�������Ҹ���ˮƽ���㣬Ѱ�ҵĲ����࣬
//	�������Լ�д��һ��ȫ��̬�ڴ������ǰ׺����
//	ʹ��������ԱȽ��鷳�����ǳ�����˼���ȫ����Ҫƽ̨֧�ֵĲ��֣�
//	ʹ��ʱ����Ҫ�ڸ���ƽ̨��֧��һϵ�е��ڴ������ͷ���غ�����
//	Ȼ��Ϳ����������ˣ�
//	���нӿ�ȫ����װ���ڲ����ⲿ����Ҫ֪���κνṹ����Ϣ��
//
//		��ǰģ��ȱ�ݣ����޸�������֧�ִ��Ҳ࿪ʼ�ҹ���
//	ģ������ʱ�����ԴӸ��ڵ㿪ʼѰ����Ϣ��
//	����������û����ģ�����ʵ���ϲ��Ǻܺ��߼���
//	��Ϊ��������˵��Ҫ�����һ���ļ��������������ڵ�Ŀ¼��Ӧ��Ӱ��������Ϊ�������Ǹ�Ŀ¼��
//	���磬�б������������� 
//		D:\\1\\2\\3\\4.c
//		D:\\1\\2\\3\\
//		D:\\1\\2\\
//		D:\\1\\
//	��ʱ�����Ҫ��ѯ D:\\1\\2\\3\\4.c ���·���Ĺ���
//		��ô����ȷ���ҵĻ�����ֱ���ҵ� D:\\1\\2\\3\\4.c ����·���Ĺ���
//	���������Ҫ��ѯ D:\\1\\2\\3\\5.c ���·���Ĺ���
//		����ʵ���϶�Ӧ·���Ĺ��򲻴��ڣ���ô��ȷ���Ҿͻ�ʧ��
//		������������е�ģ�����ң�ֻ�����ҵ� D:\\1\\ Ŀ¼�Ĺ���
//			����ʵ���� D:\\1\\2\\3\\ ���Ŀ¼�Ĺ������Ӧ���ҵ���
//
//	Ŀǰ�����ǲ����������㣬д�˴󲿷�������������Щ��Ȼ����

#pragma once
typedef unsigned long ULONG;
typedef unsigned short WCHAR;
typedef void * PVOID;
#ifndef NULL
#define NULL    ((void *)0)
#endif


typedef unsigned short TRIE_INDEX;



//	����һ��TRIE��������ǻ����������ṩ
typedef PVOID (*ZooTypeFunc_TrieTree_AllocTrieMemory)(ULONG uLen);

//	����һ��TRIE��������ǻ����������ṩ
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyTrieMemory)(PVOID p);

//	����һ��DATA�أ����ҷ���
//		���DATA�أ��������Լ��õ�
typedef PVOID (*ZooTypeFunc_TrieTree_AllocDataPool)(PVOID pTrieTree);

//	DATA�������뺯��
//		����Ҫ���������ǣ�
//		���� uLen ���Ԫ�صĴ�С��
//		�� pool ����������һ���ڴ������
//		��������ڴ�֮�󣬰�data ������ȥ��
//		һϵ�ж��ɹ��ˣ��򷵻�������ڴ棬���򷵻�NULL
typedef PVOID (*ZooTypeFunc_TrieTree_AllocData)(PVOID pTrieTree, PVOID pool, ULONG uLen, PVOID data);

//	DATA�����ͷź���
//		����Ҫ���������ǣ�
//		�� pTrieTree->poolData �����棬�� data �ͷŵ���
//		�ͷųɹ��ˣ�����NULL��������Ȼ����data
typedef PVOID (*ZooTypeFunc_TrieTree_FreeData)(PVOID pTrieTree, PVOID pool, PVOID data);

//	����DATA pool
//		����Ҫ���������ǣ�
//		�����ͷ����� pTrieTree->poolData
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyData)(PVOID pTrieTree, PVOID *pool);

//	����һ��NODE�أ����ҷ���
//		���NODE�أ��������Լ��õ�
typedef PVOID (*ZooTypeFunc_TrieTree_AllocNodePool)(PVOID pTrieTree);

//	NODE�������뺯��
//		����Ҫ���������ǣ�
//		���� sizeof(TRIE_TREE_NODE) �Ĵ�С��
//		�� pTrieTree->poolNode ����������һ���ڴ������
//		����ɹ����򷵻�������ڴ棬���򷵻�NULL
typedef PVOID (*ZooTypeFunc_TrieTree_AllocNode)(PVOID pTrieTree, PVOID pool, ULONG uLen);

//	NODE�����ͷź���
//		����ʲô�����������ͺ���
typedef PVOID (*ZooTypeFunc_TrieTree_FreeNode)(PVOID pTrieTree, PVOID pool, PVOID data);

//	����NODE pool
//		����Ҫ���������ǣ�
//		�����ͷ����� pTrieTree->poolNode
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyNode)(PVOID pTrieTree, PVOID *pool);

//	��ǰ���������ڻ�ȡָ�������ʱ���ж϶��Ŀ¼�ж�������ʱ�����ĸ�����
//	���磬������
//	D:\\1\\2\\3\\4.c
//	D:\\1\\2\\3\\
//	D:\\1\\2\\
//	D:\\1\\
//	�����������򣬵����� TrieTree_GetSpecifyRuleCmp ��ȡ����ʱ
//	����ҵ�Key �ǣ� D:\\1\\2\\3\\4.c
//	��ô����������ʱ�򣬾ͻᴥ����ǰ���� TrieTree_GetSpecifyRuleCmp
//	��һ�Σ�p ָ�� D:\\1\\ �Ĺ�������������û���������ǵ� key���Ǿͼ���
//	�ڶ��Σ�p ָ�� D:\\1\\2\\ �Ĺ���
//	�����Σ�p ָ�� D:\\1\\2\\3\\ �Ĺ���
//	���Ĵ����У�p ָ�� D:\\1\\2\\3\\4.c �Ĺ���
typedef ULONG (*ZooTypeFunc_TrieTree_GetSpecifyRuleCmp)(PVOID pThis, WCHAR *wsKey, PVOID pParam, PVOID pData);

//	�ڴ����ú�����memset
typedef PVOID (*ZooTypeFunc_TrieTree_MemorySet)(PVOID pTrieTree, PVOID _Dst, ULONG _Val, ULONG _Size);


typedef struct _TRIE_TREE_MEMORY_FUNCTION
{
	ZooTypeFunc_TrieTree_AllocTrieMemory AllocTrieMemory;
	ZooTypeFunc_TrieTree_DestroyTrieMemory DestroyTrieMemory;

	ZooTypeFunc_TrieTree_AllocDataPool AllocDataPool;
	ZooTypeFunc_TrieTree_AllocData AllocData;
	ZooTypeFunc_TrieTree_FreeData FreeData;
	ZooTypeFunc_TrieTree_DestroyData DestroyData;

	ZooTypeFunc_TrieTree_AllocNodePool AllocNodePool;
	ZooTypeFunc_TrieTree_AllocNode AllocNode;
	ZooTypeFunc_TrieTree_FreeNode FreeNode;
	ZooTypeFunc_TrieTree_DestroyNode DestroyNode;

	ZooTypeFunc_TrieTree_MemorySet MemSet;

	ZooTypeFunc_TrieTree_GetSpecifyRuleCmp GetSpecifyRuleCmp;
}TRIE_TREE_MEMORY_FUNCTION, *PTRIE_TREE_MEMORY_FUNCTION;


//	TrieTree ��ʼ��
//		ԭ����˵�����ȳ�ʼ��Ȼ�����ã�
//			�����б��У���Щ�����Ǳ���Ҫ�õ���
//			plw ������������ֶεı�ʶ���飬���֧��10����
//				��Ŀ¼�ĽǶ��Ͼ��� L'\\' �� L'/'
ULONG TrieTree_Init(PVOID *pTrie, ULONG uDataSize, PTRIE_TREE_MEMORY_FUNCTION pFun, TRIE_INDEX* plw, ULONG uLwCount);


//	TrieTree ������
ULONG TrieTree_Destroy(PVOID *pTrie);


//	TrieTree ��������
//		����һ��Key �� data ���뵽 TrieTree ����
ULONG TrieTree_Insert(PVOID pTrie, WCHAR *wsKey, PVOID data);


//	TrieTree �޸�����
//		��ȷ�޸� TrieTree ��ָ�� Key �����ݣ����Key�����ڣ�������������Ǿ�ȷ�޸�
//		��� data ��ֵΪNULL �Ļ�����ô����ͺ�ɾ������û��������
ULONG TrieTree_Revise(PVOID pTrie, WCHAR *wsKey, PVOID data);


//	TrieTree ��������
//		����wsKey ��ȷ����Ԫ�أ��ҵ��Ͱ���Ϣ���������Ҳ�����ʧ��
ULONG TrieTree_Search(PVOID pTrie, WCHAR *wsKey, PVOID *pdata);


//	TrieTree ɾ������
//		ɾ����ʱ���������Ҫ�ѽڵ㶼ɾ�ˣ�Ҳ������ô�����ֽڣ�ֻҪ��������Ĩ���ͺ���
//		Ȼ�����ҵ������ʱ�򣬵�ǰKey ��û��������
ULONG TrieTree_Delete(PVOID pTrie, WCHAR *wsKey);


//	TrieTree ��������
//		��������Ǹ�ģ�����ң����ݹ���һ��һ�β��ң�Ŀǰ�Ĺ����� \ �� / ���õ�
//		ֻҪ�ҵ���ǰKey ��������Key �����ҹ���ֶη�ʽ��ͬ������Ϊ�ҵ�����
//	���磺
//		TrieTree �ڲ���һ��Key Ϊ D:\\1\\
//		Ҫ�ҵ�Key Ϊ D:\\1\\2\\3.c
//		����Ҫ����Ϊ D:\\1\\ �����ݣ���������Ѱ�� D:\\1\\2\\3.c ��ʱ�������� D:\\1\\ �ͻ�ֱ�ӷ�������
ULONG TrieTree_GetRule(PVOID pTrie, WCHAR *wsKey, PVOID *pv);


//	TrieTree ��������
//		�������Ҳ�Ǹ�ģ�����ң���������֧�ֵ����ⲿ�ṩƥ�亯�����жϲ��ҽ��
//		��������ܿ���֧�ֶ�����
//		��������� TrieTree_GetSpecifyRuleCmpFunc �Ķ���
ULONG TrieTree_GetSpecifyRuleCmp(PVOID pTrie, WCHAR *wsKey, PVOID pParam);


//	TrieTree ��������
//		��������Ǹ�ģ�����ң����ݹ���һ��һ�β��ң�Ŀǰ�Ĺ����� \ �� / ���õ�
//		ֻҪ�ҵ���ǰKey ��������Key �����ҹ���ֶη�ʽ��ͬ������Ϊ�ҵ�����
//		���Ҳ࿪ʼ��
ULONG TrieTree_GetRule_Right(PVOID pTrie, WCHAR *wsKey, PVOID *pv);


//	TrieTree ��������
//		�������Ҳ�Ǹ�ģ�����ң���������֧�ֵ����ⲿ�ṩƥ�亯�����жϲ��ҽ��
//		��������ܿ���֧�ֶ�����
//		��������� TrieTree_GetSpecifyRuleCmpFunc �Ķ���
//		���Ҳ࿪ʼ��
ULONG TrieTree_GetSpecifyRuleCmp_Right(PVOID pTrie, WCHAR *wsKey, PVOID pParam);


//	TrieTree ��ȡ����ʹ�õ�node����
ULONG TrieTree_GetNodeOnUseCount(PVOID pTrie);


//	TrieTree ��ȡ����node����
ULONG TrieTree_GetAllocNodeCount(PVOID pTrie);


//	TrieTree ��ȡ����ʹ�õ�Data����
ULONG TrieTree_GetDataOnUseCount(PVOID pTrie);


//	TrieTree ��ȡ����Data����
ULONG TrieTree_GetAllocDataCount(PVOID pTrie);


//	TrieTree ��ȡ�ڵ��С
ULONG TrieTree_GetNodeSize(PVOID pTrie);


//	TrieTree ��ȡData��С
ULONG TrieTree_GetDataSize(PVOID pTrie);