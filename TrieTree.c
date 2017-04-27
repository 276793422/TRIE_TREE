//	Author: Zoo
//	QQ:276793422
//
#include "TrieTree.h"

#pragma pack(push , 1)

typedef struct _TRIE_TREE_NODE TRIE_TREE_NODE, *PTRIE_TREE_NODE;

typedef struct _TRIE_TREE_NODE
{
	TRIE_INDEX name;			//	当前节点索引
	PTRIE_TREE_NODE parent;		//	父节点指针，指的是上一个节点
	PTRIE_TREE_NODE next;		//	自己后面的索引
	PTRIE_TREE_NODE subHead;	//	子索引头
	PVOID data;					//	当前节点数据
};

typedef struct _TRIE_TREE TRIE_TREE, *PTRIE_TREE;

typedef struct _TRIE_TREE
{
	PTRIE_TREE_NODE pTrieHead;

	//	这里需要一个内存池，这个内存池用来分配所有当前表的内存
	//		表销毁的时候，实际上内存是不会被释放的，
	//		没必要浪费时间，也不需要节省那么几十个字节，
	//		在最终 TRIE 销毁的时候，可以直接全部释放
	PVOID poolNode;
	ULONG allocNodeOnUse;	//	正在使用的个数
	ULONG allocNodeTimes;	//	node节点申请次数

	PVOID poolData;			//	DATA数据申请池，为了更好的管理内存。
	ULONG allocDataOnUse;	//	正在使用的个数
	ULONG allocDataTimes;	//	data节点申请次数
	ULONG uDataSize;		//	DATA数据大小，这个大小应该是固定的。

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



//	这个函数的目的就是如果p有值就返回，没值就给它创建个值返回，保证它一定有值
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
		//	到 Key 的结尾了，并且也到了tree 结尾了
		if (*wsKey == L'\0' && pthis->name == L'\0')
		{
			//	原来就有
			if (pthis->data)
			{
				break;
			}
			pthis->data = TrieTree_AllocData(pTrieTree, pTrieTree->poolData, data);
			bRet = 1;
			break;
		}

		//	是当前前缀
		if (pthis->name == *wsKey)
		{
			pThis = &pthis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
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
		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			pThis->data = TrieTree_FreeData(pTrieTree, pTrieTree->poolData, pThis->data);
			pThis->data = TrieTree_AllocData(pTrieTree, pTrieTree->poolData, data);
			bRet = 1;
			break;
		}
		//	是当前前缀
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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
		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
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
		//	是当前前缀
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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

//	删除的时候根本不需要把节点都删了，也不差那么几个字节，只要把数据区抹掉就好了
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
		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			pThis->data = TrieTree_FreeData(pTrieTree, pTrieTree->poolData, pThis->data);
			bRet = 1;
			break;
		}
		//	是当前前缀
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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
		//	目录只接受结尾是 \ 或者 / 的
		//	如果上层前缀是 \ 或者 /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, wLastName))
		{
			pTmp = pThis;
			//	循环判断当前层是否有 L'\0'，
			//	如果有，则认为当前 L'\0' 索引记录了当前目录对应的规则
			//	目录规则优先级低于文件规则
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


		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
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

		//	是当前前缀
		wLastName = pThis->name;
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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
		//	目录只接受结尾是 \ 或者 / 的
		//	如果上层前缀是 \ 或者 /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, wLastName))
		{
			pTmp = pThis;
			//	循环判断当前层是否有 L'\0'，
			//	如果有，则认为当前 L'\0' 索引记录了当前目录对应的规则
			//	目录规则优先级低于文件规则
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
				//	当前层有规则，并且匹配成功了
				if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pTmp->data))
				{
					bRet = 1;
					break;	
				}
			}
		}


		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
		if ((*wsKey == L'\0') && (pThis->name == L'\0'))
		{
			if (pThis->data == NULL)
			{
				break;
			}
			//	最后一条规则，匹配成功
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

		//	是当前前缀
		wLastName = pThis->name;
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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

//	TrieTree 查找数据
//		这个查找是个模糊查找，根据规则一段一段查找，目前的规则是 \ 和 / 内置的
//		只要找到当前Key 所包含的Key ，并且规则分段方式相同，则认为找到数据
//		这个查找是从右边开始找
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
		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
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
		//	是当前前缀
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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
		//	父节点不存在，也就是走到根节点
		if (pThis->parent == NULL)
		{
			break;
		}
		//	如果父节点符合要求 \ 或者 /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, pThis->parent->name))
		{
			pTmp = pThis;
			//	循环判断当前层是否有 L'\0'，
			//	如果有，则认为当前 L'\0' 索引记录了当前目录对应的规则
			//	目录规则优先级低于文件规则
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
				//	当前层有规则，并且匹配成功了
				*pdata = pTmp->data;
				bRet = 1;
				break;
			}
		}
		//	不符合要求，或者不存在规则
		pThis = pThis->parent;
	} while (1);
	return bRet;
}

//	TrieTree 查找数据
//		这个查找也是个模糊查找，但是这里支持的是外部提供匹配函数来判断查找结果
//		所以这里很可能支持多个结果
//		具体情况看 TrieTree_GetSpecifyRuleCmpFunc 的定义
//		这个查找是从右边开始找
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
		//	只有 name 为 L'\0'的子节点，里面记录的才是正确信息
		if (*wsKey == L'\0' && pThis->name == L'\0')
		{
			if (pThis->data == NULL)
			{
				break;
			}
			bRet = 1;
			break;
		}
		//	是当前前缀
		if (pThis->name == *wsKey)
		{
			pThis = pThis->subHead;
			wsKey++;
			continue;
		}
		//	不是当前前缀，找下一个
		else
		{
			//	没有下一个了，就失败了
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
		//	最后一条规则，匹配成功
		if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pThis->data))
		{
			return bRet;
		}
	}

	do 
	{
		//	父节点不存在，也就是走到根节点
		if (pThis->parent == NULL)
		{
			break;
		}
		//	如果父节点符合要求 \ 或者 /
		if (TrieTree_IsLastWordIsSpecify(pTrieTree, pThis->parent->name))
		{
			//	当前存在规则，就出去
			if (pThis->data != NULL)
			{
			}
			pTmp = pThis;
			//	循环判断当前层是否有 L'\0'，
			//	如果有，则认为当前 L'\0' 索引记录了当前目录对应的规则
			//	目录规则优先级低于文件规则
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
				//	判断当前规则
				if (pTrieTree->TrieFunc.GetSpecifyRuleCmp(pTrieTree, pwsKey, pParam, pTmp->data))
				{
					bRet = 1;
					break;
				}
			}
		}
		//	不符合要求，或者不存在规则
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

//	TrieTree 获取正在使用的node个数
ULONG TrieTree_GetNodeOnUseCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocNodeOnUse;
}


//	TrieTree 获取申请node次数
ULONG TrieTree_GetAllocNodeCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocNodeTimes;
}


//	TrieTree 获取正在使用的Data个数
ULONG TrieTree_GetDataOnUseCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocDataOnUse;
}


//	TrieTree 获取申请Data次数
ULONG TrieTree_GetAllocDataCount(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->allocDataTimes;
}

//	TrieTree 获取节点大小
ULONG TrieTree_GetNodeSize(PVOID pTrie)
{
	return sizeof(TRIE_TREE_NODE);
}

//	TrieTree 获取节点大小
ULONG TrieTree_GetDataSize(PVOID pTrie)
{
	PTRIE_TREE pTrieTree = (PTRIE_TREE)pTrie;
	return pTrieTree->uDataSize;
}