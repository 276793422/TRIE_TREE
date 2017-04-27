//	Author: Zoo
//	QQ:276793422
//
#include "TrieTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char CHAR;

//	创建一个TRIE树，这个是基础，必须提供
PVOID AllocTrieMemory(ULONG uLen)
{
	printf("[%s] \n", __FUNCTION__);
	return malloc(uLen);
}

//	销毁一个TRIE树，这个是基础，必须提供
PVOID DestroyTrieMemory(PVOID p)
{
	printf("[%s] \n", __FUNCTION__);
	free(p);
	return NULL;
}

ULONG GetSpecifyRuleCmpFunc(PVOID pTrieTree, WCHAR *wsKey, PVOID pParam, PVOID pData)
{
	printf("GetSpecifyRuleCmpFunc , pParam = %d, wsKey = %S , p = %s \n", pParam, wsKey, pData);
	return 1;
}

PVOID AllocDataPool(PVOID pTrieTree)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}

PVOID AllocData(PVOID pTrieTree, PVOID pool, ULONG uLen, PVOID data)
{
	printf("[%s] \n", __FUNCTION__);
	return data;
}

PVOID FreeData(PVOID pTrieTree, PVOID pool, PVOID data)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}

PVOID DestroyData(PVOID pTrieTree)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}



PVOID AllocNodePool(PVOID pTrieTree)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}

PVOID AllocNode(PVOID pTrieTree, PVOID pool, ULONG uLen)
{
	PVOID p = NULL;
	printf("[%s] \n", __FUNCTION__);
	p = malloc(uLen);
	if (p)
	{
		memset(p, 0, uLen);
	}
	return p;
}

PVOID FreeNode(PVOID pTrieTree, PVOID pool, PVOID data)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}

PVOID DestroyNode(PVOID pTrieTree)
{
	printf("[%s] \n", __FUNCTION__);
	return NULL;
}

PVOID MemorySet(PVOID pTrieTree, PVOID _Dst, ULONG _Val, ULONG _Size)
{
	return memset(_Dst, _Val, _Size);
}

int main()
{
	CHAR *str;
	PVOID pTrieTree = NULL;
	TRIE_TREE_MEMORY_FUNCTION Funa;
	WCHAR lwArray[10] = { L'\\', L'/' };
	Funa.AllocTrieMemory	= AllocTrieMemory;
	Funa.DestroyTrieMemory	= DestroyTrieMemory;
	Funa.AllocData			= AllocData;
	Funa.AllocDataPool		= AllocDataPool;
	Funa.AllocNode			= AllocNode;
	Funa.AllocNodePool		= AllocNodePool;
	Funa.DestroyData		= DestroyData;
	Funa.DestroyNode		= DestroyNode;
	Funa.FreeData			= FreeData;
	Funa.FreeNode			= FreeNode;
	Funa.MemSet				= MemorySet;
	Funa.GetSpecifyRuleCmp	= GetSpecifyRuleCmpFunc;

	TrieTree_Init(&pTrieTree, 4, &Funa, lwArray, 2);

	TrieTree_Insert(pTrieTree, L"D:\\1\\3\\main.c", "D:\\1\\3\\main.c");
	if (TrieTree_GetRule(pTrieTree, L"D:\\1\\3\\main.c", &str))
	{
		printf("del get str = %s \n", str);
	}
	else
	{
		printf("del get find error \n");
	}
	TrieTree_Insert(pTrieTree, L"D:\\1\\3\\", "D:\\1\\3\\");
	TrieTree_Insert(pTrieTree, L"D:\\1\\", "D:\\1\\");
	TrieTree_Insert(pTrieTree, L"D:\\DLL.c", "D:\\DLL.c");


	if (TrieTree_GetSpecifyRuleCmp(pTrieTree, L"D:\\1\\3\\main.c", (PVOID)1))
	{
		printf("str = %s \n", str);
	}

	if (TrieTree_Search(pTrieTree, L"D:\\1\\3\\main.c", &str))
	{
		printf("str = %s \n", str);
	}
	if (TrieTree_Search(pTrieTree, L"D:\\1\\3\\", &str))
	{
		printf("str = %s \n", str);
	}
	if (TrieTree_Search(pTrieTree, L"D:\\DLL.c", &str))
	{
		printf("str = %s \n", str);
	}
	if (TrieTree_Search(pTrieTree, L"D:\\1\\3\\main.c", &str))
	{
		printf("str = %s \n", str);
	}

	if (TrieTree_Delete(pTrieTree, L"D:\\1\\3\\main.c"))
	{
		if (TrieTree_Search(pTrieTree, L"D:\\1\\3\\main.c", &str))
		{
			printf("del search str = %s \n", str);
		}
		else
		{
			printf("del search find error \n");
		}
		if (TrieTree_GetRule(pTrieTree, L"D:\\1\\3\\main.c", &str))
		{
			printf("del get str = %s \n", str);
		}
		else
		{
			printf("del get find error \n");
		}
		if (TrieTree_GetRule_Right(pTrieTree, L"D:\\1\\3\\main.c", &str))
		{
			printf("del get str = %s \n", str);
		}
		else
		{
			printf("del get find error \n");
		}
	}

	if (TrieTree_Delete(pTrieTree, L"D:\\1\\"))
	{
		if (TrieTree_GetRule(pTrieTree, L"D:\\1\\3\\main.c", &str))
		{
			printf("del get str = %s \n", str);
		}
		else
		{
			printf("del get find error \n");
		}
	}

	printf("TrieTree_GetNodeOnUseCount() = %d \n", TrieTree_GetNodeOnUseCount(pTrieTree));
	printf("TrieTree_GetAllocNodeCount() = %d \n", TrieTree_GetAllocNodeCount(pTrieTree));
	printf("TrieTree_GetDataOnUseCount() = %d \n", TrieTree_GetDataOnUseCount(pTrieTree));
	printf("TrieTree_GetAllocDataCount() = %d \n", TrieTree_GetAllocDataCount(pTrieTree));
	printf("TrieTree_GetNodeSize() = %d \n", TrieTree_GetNodeSize(pTrieTree));
	printf("TrieTree_GetDataSize() = %d \n", TrieTree_GetDataSize(pTrieTree));

	TrieTree_Destroy(&pTrieTree);
	return 0;
}