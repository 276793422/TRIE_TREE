//	Author: Zoo
//	QQ:276793422
//
#include "TrieTree.h"

#pragma pack(push , 1)

typedef struct _TRIE_TREE_NODE TRIE_TREE_NODE, *PTRIE_TREE_NODE;

typedef struct _TRIE_TREE_NODE
{
	TRIE_INDEX name;			//	��ǰ�ڵ�����
	PTRIE_TREE_NODE parent;		//	���ڵ�ָ�룬ָ������һ���ڵ�
	PTRIE_TREE_NODE next;		//	�Լ����������
	PTRIE_TREE_NODE subHead;	//	������ͷ
	PVOID data;					//	��ǰ�ڵ�����
};

typedef struct _TRIE_TREE TRIE_TREE, *PTRIE_TREE;

typedef struct _TRIE_TREE
{
	PTRIE_TREE_NODE pTrieHead;

	//	������Ҫһ���ڴ�أ�����ڴ�������������е�ǰ����ڴ�
	//		�����ٵ�ʱ��ʵ�����ڴ��ǲ��ᱻ�ͷŵģ�
	//		û��Ҫ�˷�ʱ�䣬Ҳ����Ҫ��ʡ��ô��ʮ���ֽڣ�
	//		������ TRIE ���ٵ�ʱ�򣬿���ֱ��ȫ���ͷ�
	PVOID poolNode;
	ULONG allocNodeOnUse;	//	����ʹ�õĸ���
	ULONG allocNodeTimes;	//	node�ڵ��������

	PVOID poolData;			//	DATA��������أ�Ϊ�˸��õĹ����ڴ档
	ULONG allocDataOnUse;	//	����ʹ�õĸ���
	ULONG allocDataTimes;	//	data�ڵ��������
	ULONG uDataSize;		//	DATA���ݴ�С�������СӦ���ǹ̶��ġ�

	TRIE_TREE_MEMORY_FUNCTION TrieFunc;

	TRIE_INDEX lastWord[10];
	ULONG lastWordCount;
};
#pragma pack(pop)


static PVOID TrieTree_AllocDataPool(PTRIE_TREE pTrieTree)
{
	if (pTrieTree->TrieFunc.AllocDataPool)
	{
		return pTrieTree->TrieFunc.AllocDataPool(pTrieTree);
	}
	else
	{
		return NULL;
	}
}

static PVOID TrieTree_AllocData(PTRIE_TREE pTrieTree, PVOID pool, PVOID data)
{
	if (pTrieTree->TrieFunc.AllocData)
	{
		PVOID pv = pTrieTree->TrieFunc.AllocData(pTrieTree, pool, pTrieTree->uDataSize, data);
		if (pv)
		{
			pTrieTree->allocDataOnUse ++;
			pTrieTree->allocDataTimes ++;
		}
		return pv;
	}
	else
	{
		return data;
	}
}

static PVOID TrieTree_FreeData(PTRIE_TREE pTrieTree, PVOID pool, PVOID data)
{
	if (pTrieTree->TrieFunc.FreeData)
	{
		PVOID pv = pTrieTree->TrieFunc.FreeData(pTrieTree, pool, data);
		if (pv == NULL)
		{
			pTrieTree->allocDataOnUse --;
		}
		return pv;
	}
	else
	{
		return NULL;
	}
}

static PVOID TrieTree_DestroyData(PTRIE_TREE pTrieTree)
{
	if (pTrieTree->TrieFunc.DestroyData)
	{
		return pTrieTree->TrieFunc.DestroyData(pTrieTree);
	}
	else
	{
		return NULL;
	}
}



static PVOID TrieTree_AllocNodePool(PTRIE_TREE pTrieTree)
{
	if (pTrieTree->TrieFunc.AllocNodePool)
	{
		return pTrieTree->TrieFunc.AllocNodePool(pTrieTree);
	}
	else
	{
		return NULL;
	}
}

static PVOID TrieTree_AllocNode(PTRIE_TREE pTrieTree, PVOID pool)
{
	if (pTrieTree->TrieFunc.AllocNode)
	{
		PVOID pv = pTrieTree->TrieFunc.AllocNode(pTrieTree, pool, sizeof(TRIE_TREE_NODE));
		if (pv)
		{
			pTrieTree->allocNodeOnUse ++;
			pTrieTree->allocNodeTimes ++;
		}
		return pv;
	}
	else
	{
		return NULL;
	}
}

static PVOID TrieTree_FreeNode(PTRIE_TREE pTrieTree, PVOID pool, PVOID data)
{
	if (pTrieTree->TrieFunc.FreeNode)
	{
		PVOID pv = pTrieTree->TrieFunc.FreeNode(pTrieTree, pool, data);
		if (pv == NULL)
		{
			pTrieTree->allocNodeOnUse --;
		}
		return pv;
	}
	else
	{
		return NULL;
	}
}

static PVOID TrieTree_DestroyNode(PTRIE_TREE pTrieTree)
{
	if (pTrieTree->TrieFunc.DestroyNode)
	{
		return pTrieTree->TrieFunc.DestroyNode(pTrieTree);
	}
	else
	{
		return NULL;
	}
}



//	���������Ŀ�ľ������p��ֵ�ͷ��أ�ûֵ�͸���������ֵ���أ���֤��һ����ֵ
static PTRIE_TREE_NODE TrieTree_AllocSpace(PTRIE_TREE pTrieTree, PTRIE_TREE_NODE p, WCHAR wChar)
{
	if (p == NULL)
	{
		p = TrieTree_AllocNode(pTrieTree, pTrieTree->poolNode);
		if (p == NULL)
		{
			return NULL;
		}
		pTrieTree->TrieFunc.MemSet(pTrieTree, p, 0, sizeof(*p));
		p->name = wChar;
	}
	return p;
}

static ULONG TrieTree_IsLastWordIsSpecify(PTRIE_TREE pTrieTree, ULONG wChar)
{
	ULONG i;
	for (i = 0; i < pTrieTree->lastWordCount; i++)
	{
		if (wChar == pTrieTree->lastWord[i])
		{
			return 1;
		}
	}
	return 0;
}


ULONG TrieTree_Init(PVOID *pTrie, ULONG uDataSize, PTRIE_TREE_MEMORY_FUNCTION pFun, TRIE_INDEX* plw, ULONG uLwCount)
{
	PTRIE_TREE pTrieTree = NULL;
	if (pFun == NULL)
	{
		return 0;
	}
	if (pFun->AllocNode == NULL)
	{
		return 0;
	}
	if (pFun->MemSet == NULL)
	{
		return 0;
	}
	if (pFun->AllocTrieMemory == NULL)
	{
		return 0;
	}
	if (plw == NULL)
	{
		return 0;
	}
	if (uLwCount < 0 || 10 < uLwCount)
	{
		return 0;
	}
	pTrieTree = pFun->AllocTrieMemory(sizeof(TRIE_TREE));
	if (pTrieTree == NULL)
	{
		return 0;
	}
	pFun->MemSet(NULL, pTrieTree, 0, sizeof(*pTrieTree));
	pTrieTree->uDataSize = uDataSize;
	pTrieTree->TrieFunc = *pFun;
	pTrieTree->poolNode = TrieTree_AllocNodePool(pTrieTree);
	pTrieTree->poolData = TrieTree_AllocDataPool(pTrieTree);

	pTrieTree->lastWordCount = uLwCount;
	for (uLwCount--; uLwCount < 10; uLwCount--)
	{
		pTrieTree->lastWord[uLwCount] = plw[uLwCount];
	}
	*pTrie = pTrieTree;
	return 1;
}

ULONG TrieTree_Insert(PVOID pTrie, WCHAR *wsKey, PVOID data)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE *pThis = &(pTrieTree->pTrieHead);
	PTRIE_TREE_NODE pthis = NULL;
	ULONG bRet = 0;

	if (pTrieTree == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	if (data == NULL)
	{
		return bRet;
	}
	do 
	{
		*pThis = TrieTree_AllocSpace(pTrieTree, *pThis, *wsKey);
		(*pThis)->parent = pthis;
		pthis = *pThis;
		if (*pThis == NULL)
		{
			break;
		}
		//	�� Key �Ľ�β�ˣ�����Ҳ����tree ��β��
		if (*wsKey == L'\0' && pthis->name == L'\0')
		{
			//	ԭ������
			if (pthis->data)
			{
				break;
			}
			pthis->data = TrieTree_AllocData(pTrieTree, pTrieTree->poolData, data);
			bRet = 1;
			break;
		}

		//	�ǵ�ǰǰ׺
		if (pthis->name == *wsKey)
		{
			pThis = &pthis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			pThis = &pthis->next;
			wsKey;
			continue;
		}
	} while (1);
	return bRet;
}

ULONG TrieTree_Revise(PVOID pTrie, WCHAR *wsKey, PVOID data)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	do 
	{
		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			pThis->data = TrieTree_FreeData(pTrieTree, pTrieTree->poolData, pThis->data);
			pThis->data = TrieTree_AllocData(pTrieTree, pTrieTree->poolData, data);
			bRet = 1;
			break;
		}
		//	�ǵ�ǰǰ׺
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);
	return bRet;
}

ULONG TrieTree_Search(PVOID pTrie, WCHAR *wsKey, PVOID *pdata)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	if (pdata == NULL)
	{
		return bRet;
	}
	do 
	{
		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			if (pThis->data == NULL)
			{
				break;
			}
			*pdata = pThis->data;
			bRet = 1;
			break;
		}
		//	�ǵ�ǰǰ׺
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);
	return bRet;
}

//	ɾ����ʱ���������Ҫ�ѽڵ㶼ɾ�ˣ�Ҳ������ô�����ֽڣ�ֻҪ��������Ĩ���ͺ���
ULONG TrieTree_Delete(PVOID pTrie, WCHAR *wsKey)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	do 
	{
		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			pThis->data = TrieTree_FreeData(pTrieTree, pTrieTree->poolData, pThis->data);
			bRet = 1;
			break;
		}
		//	�ǵ�ǰǰ׺
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);
	return bRet;
}

ULONG TrieTree_GetRule(PVOID pTrie, WCHAR *wsKey, PVOID *pv)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	PTRIE_TREE_NODE pTmp;
	WCHAR wLastName = L'\0';
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	if (pv == NULL)
	{
		return bRet;
	}
	do 
	{
		//	Ŀ¼ֻ���ܽ�β�� \ ���� / ��
		//	����ϲ�ǰ׺�� \ ���� /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, wLastName))
		{
			pTmp = pThis;
			//	ѭ���жϵ�ǰ���Ƿ��� L'\0'��
			//	����У�����Ϊ��ǰ L'\0' ������¼�˵�ǰĿ¼��Ӧ�Ĺ���
			//	Ŀ¼�������ȼ������ļ�����
			while (pTmp)
			{
				if (pTmp->name == L'\0')
				{
					break;
				}
				else
				{
					pTmp = pTmp->next;
				}
			}
			if (pTmp && (pTmp->name == L'\0') && (pTmp->data != NULL))
			{
				*pv = pTmp->data;
				bRet = 1;
				break;
			}
		}


		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if ((*wsKey == L'\0') && (pThis->name == L'\0'))
		{
			if (pThis->data == NULL)
			{
				break;
			}
			*pv = pThis->data;
			bRet = 1;
			break;
		}

		//	�ǵ�ǰǰ׺
		wLastName = pThis->name;
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);
	return bRet;
}

ULONG TrieTree_GetSpecifyRuleCmp(PVOID pTrie, WCHAR *wsKey, PVOID pParam)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	PTRIE_TREE_NODE pTmp;
	WCHAR wLastName = L'\0';
	WCHAR *pwsKey = wsKey;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	if (pParam == NULL)
	{
		return bRet;
	}
	do 
	{
		//	Ŀ¼ֻ���ܽ�β�� \ ���� / ��
		//	����ϲ�ǰ׺�� \ ���� /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, wLastName))
		{
			pTmp = pThis;
			//	ѭ���жϵ�ǰ���Ƿ��� L'\0'��
			//	����У�����Ϊ��ǰ L'\0' ������¼�˵�ǰĿ¼��Ӧ�Ĺ���
			//	Ŀ¼�������ȼ������ļ�����
			while (pTmp)
			{
				if (pTmp->name == L'\0')
				{
					break;
				}
				else
				{
					pTmp = pTmp->next;
				}
			}
			if (pTmp && (pTmp->name == L'\0') && (pTmp->data != NULL))
			{
				//	��ǰ���й��򣬲���ƥ��ɹ���
				if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pTmp->data))
				{
					bRet = 1;
					break;	
				}
			}
		}


		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if ((*wsKey == L'\0') && (pThis->name == L'\0'))
		{
			if (pThis->data == NULL)
			{
				break;
			}
			//	���һ������ƥ��ɹ�
			if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pThis->data))
			{
				bRet = 1;
				break;	
			}
			else
			{
				break;
			}
		}

		//	�ǵ�ǰǰ׺
		wLastName = pThis->name;
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);
	return bRet;
}

//	TrieTree ��������
//		��������Ǹ�ģ�����ң����ݹ���һ��һ�β��ң�Ŀǰ�Ĺ����� \ �� / ���õ�
//		ֻҪ�ҵ���ǰKey ��������Key �����ҹ���ֶη�ʽ��ͬ������Ϊ�ҵ�����
//		��������Ǵ��ұ߿�ʼ��
ULONG TrieTree_GetRule_Right(PVOID pTrie, WCHAR *wsKey, PVOID *pdata)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	PTRIE_TREE_NODE pTmp;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	if (pdata == NULL)
	{
		return bRet;
	}
	do 
	{
		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			if (pThis->data == NULL)
			{
				break;
			}
			*pdata = pThis->data;
			bRet = 1;
			break;
		}
		//	�ǵ�ǰǰ׺
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);

	if (bRet)
	{
		return bRet;
	}

	do 
	{
		//	���ڵ㲻���ڣ�Ҳ�����ߵ����ڵ�
		if (pThis->parent == NULL)
		{
			break;
		}
		//	������ڵ����Ҫ�� \ ���� /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, pThis->parent->name))
		{
			pTmp = pThis;
			//	ѭ���жϵ�ǰ���Ƿ��� L'\0'��
			//	����У�����Ϊ��ǰ L'\0' ������¼�˵�ǰĿ¼��Ӧ�Ĺ���
			//	Ŀ¼�������ȼ������ļ�����
			while (pTmp)
			{
				if (pTmp->name == L'\0')
				{
					break;
				}
				else
				{
					pTmp = pTmp->next;
				}
			}
			if (pTmp && (pTmp->name == L'\0') && (pTmp->data != NULL))
			{
				//	��ǰ���й��򣬲���ƥ��ɹ���
				*pdata = pTmp->data;
				bRet = 1;
				break;
			}
		}
		//	������Ҫ�󣬻��߲����ڹ���
		pThis = pThis->parent;
	} while (1);
	return bRet;
}

//	TrieTree ��������
//		�������Ҳ�Ǹ�ģ�����ң���������֧�ֵ����ⲿ�ṩƥ�亯�����жϲ��ҽ��
//		��������ܿ���֧�ֶ�����
//		��������� TrieTree_GetSpecifyRuleCmpFunc �Ķ���
//		��������Ǵ��ұ߿�ʼ��
ULONG TrieTree_GetSpecifyRuleCmp_Right(PVOID pTrie, WCHAR *wsKey, PVOID pParam)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	PTRIE_TREE_NODE pThis = pTrieTree->pTrieHead;
	PTRIE_TREE_NODE pTmp;
	WCHAR *pwsKey = wsKey;
	ULONG bRet = 0;
	if (pThis == NULL)
	{
		return bRet;
	}
	if (wsKey == NULL)
	{
		return bRet;
	}
	do 
	{
		//	ֻ�� name Ϊ L'\0'���ӽڵ㣬�����¼�Ĳ�����ȷ��Ϣ
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			if (pThis->data == NULL)
			{
				break;
			}
			bRet = 1;
			break;
		}
		//	�ǵ�ǰǰ׺
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	���ǵ�ǰǰ׺������һ��
		else
		{
			//	û����һ���ˣ���ʧ����
			if (pThis->next == NULL)
			{
				break;
			}
			pThis = pThis->next;
			continue;
		}
	} while (1);

	if (bRet)
	{
		//	���һ������ƥ��ɹ�
		if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pThis->data))
		{
			return bRet;
		}
	}

	do 
	{
		//	���ڵ㲻���ڣ�Ҳ�����ߵ����ڵ�
		if (pThis->parent == NULL)
		{
			break;
		}
		//	������ڵ����Ҫ�� \ ���� /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, pThis->parent->name))
		{
			//	��ǰ���ڹ��򣬾ͳ�ȥ
			if (pThis->data != NULL)
			{
			}
			pTmp = pThis;
			//	ѭ���жϵ�ǰ���Ƿ��� L'\0'��
			//	����У�����Ϊ��ǰ L'\0' ������¼�˵�ǰĿ¼��Ӧ�Ĺ���
			//	Ŀ¼�������ȼ������ļ�����
			while (pTmp)
			{
				if (pTmp->name == L'\0')
				{
					break;
				}
				else
				{
					pTmp = pTmp->next;
				}
			}
			if (pTmp && (pTmp->name == L'\0') && (pTmp->data != NULL))
			{
				//	�жϵ�ǰ����
				if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pTmp->data))
				{
					bRet = 1;
					break;
				}
			}
		}
		//	������Ҫ�󣬻��߲����ڹ���
		pThis = pThis->parent;
	} while (1);
	return bRet;
}

ULONG TrieTree_Destroy(PVOID *pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)*pTrie;
	pTrieTree->poolData = TrieTree_DestroyData(pTrieTree);
	pTrieTree->poolNode = TrieTree_DestroyNode(pTrieTree);
	//pTrieTree->TrieFunc.MemSet(pTrieTree, pTrieTree, 0, sizeof(*pTrieTree));
	if (pTrieTree->TrieFunc.DestroyTrieMemory(pTrieTree))
	{
		return 0;
	}
	*pTrie = NULL;
	return 1;
}

//	TrieTree ��ȡ����ʹ�õ�node����
ULONG TrieTree_GetNodeOnUseCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocNodeOnUse;
}


//	TrieTree ��ȡ����node����
ULONG TrieTree_GetAllocNodeCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocNodeTimes;
}


//	TrieTree ��ȡ����ʹ�õ�Data����
ULONG TrieTree_GetDataOnUseCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocDataOnUse;
}


//	TrieTree ��ȡ����Data����
ULONG TrieTree_GetAllocDataCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocDataTimes;
}

//	TrieTree ��ȡ�ڵ��С
ULONG TrieTree_GetNodeSize(PVOID pTrie)
{
	return sizeof(TRIE_TREE_NODE);
}

//	TrieTree ��ȡ�ڵ��С
ULONG TrieTree_GetDataSize(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->uDataSize;
}