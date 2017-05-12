//	Author: Zoo
//	QQ:276793422
//
//		模块说明
//	当前模块是一个支持跨平台的全动态申请的前缀树，
//	由于前一阵子需要用到一个前缀树，
//	本来想在网上找一个，但是我就没有找到全动态申请内存的前缀树，
//	网上的各种前缀树都是子列表元素个数确定的，
//	但是我用前缀树的目的是用来存放文件路径，
//	文件路径的字符串是包含中文字符的，
//	这样，如果把子列表元素个数确定的话，
//	那么就可能需要每个树节点都需要0xFFFF个指针元素，
//	如果真的是这样，就疯了，内存浪费过多，x86环境，一个节点就要用掉256K内存，x64下翻倍，
//	所以我的需求是找到一个全动态内存申请的前缀树，
//	可是，我找了各大平台，各大开源代码，都没有找到，
//	也可能是我个人水平不足，寻找的不够多，
//	所以我自己写了一个全动态内存申请的前缀树，
//	使用起来相对比较麻烦，但是抽象出了几乎全部需要平台支持的部分，
//	使用时，需要在各自平台上支持一系列的内存申请释放相关函数，
//	然后就可以随意用了，
//	所有接口全部封装在内部，外部不需要知道任何结构体信息。
//
//		当前模块缺陷（已修复，可以支持从右侧开始找规则）
//	模糊查找时，可以从根节点开始寻找信息，
//	这样本身是没问题的，但是实际上不是很合逻辑，
//	因为道理上来说需要如果对一个文件做操作，它所在的目录才应该影响它的行为，而不是根目录。
//	比如，列表中有四条规则 
//		D:\\1\\2\\3\\4.c
//		D:\\1\\2\\3\\
//		D:\\1\\2\\
//		D:\\1\\
//	这时如果我要查询 D:\\1\\2\\3\\4.c 这个路径的规则
//		那么，精确查找的话，会直接找到 D:\\1\\2\\3\\4.c 这条路径的规则，
//	但是如果我要查询 D:\\1\\2\\3\\5.c 这个路径的规则
//		由于实际上对应路径的规则不存在，那么精确查找就会失败
//		但是如果用现有的模糊查找，只能先找到 D:\\1\\ 目录的规则
//			但是实际上 D:\\1\\2\\3\\ 这个目录的规则才是应该找到的
//
//	目前问题是测试用例不足，写了大部分用例，但是有些仍然不够

#pragma once
typedef unsigned long ULONG;
typedef unsigned short WCHAR;
typedef void * PVOID;
#ifndef NULL
#define NULL    ((void *)0)
#endif


typedef unsigned short TRIE_INDEX;



//	创建一个TRIE树，这个是基础，必须提供
typedef PVOID (*ZooTypeFunc_TrieTree_AllocTrieMemory)(ULONG uLen);

//	销毁一个TRIE树，这个是基础，必须提供
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyTrieMemory)(PVOID p);

//	创建一个DATA池，并且返回
//		这个DATA池，是外面自己用的
typedef PVOID (*ZooTypeFunc_TrieTree_AllocDataPool)(PVOID pTrieTree);

//	DATA数据申请函数
//		这里要做的事情是：
//		根据 uLen 这个元素的大小，
//		从 pool 这里面申请一块内存出来，
//		申请出来内存之后，把data 拷贝进去，
//		一系列都成功了，则返回申请的内存，否则返回NULL
typedef PVOID (*ZooTypeFunc_TrieTree_AllocData)(PVOID pTrieTree, PVOID pool, ULONG uLen, PVOID data);

//	DATA数据释放函数
//		这里要做的事情是：
//		从 pTrieTree->poolData 这里面，把 data 释放掉，
//		释放成功了，返回NULL，否则仍然返回data
typedef PVOID (*ZooTypeFunc_TrieTree_FreeData)(PVOID pTrieTree, PVOID pool, PVOID data);

//	销毁DATA pool
//		这里要做的事情是：
//		销毁释放整个 pTrieTree->poolData
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyData)(PVOID pTrieTree, PVOID *pool);

//	创建一个NODE池，并且返回
//		这个NODE池，是外面自己用的
typedef PVOID (*ZooTypeFunc_TrieTree_AllocNodePool)(PVOID pTrieTree);

//	NODE数据申请函数
//		这里要做的事情是：
//		根据 sizeof(TRIE_TREE_NODE) 的大小，
//		从 pTrieTree->poolNode 这里面申请一块内存出来，
//		申请成功，则返回申请的内存，否则返回NULL
typedef PVOID (*ZooTypeFunc_TrieTree_AllocNode)(PVOID pTrieTree, PVOID pool, ULONG uLen);

//	NODE数据释放函数
//		这里什么都不用做，就好了
typedef PVOID (*ZooTypeFunc_TrieTree_FreeNode)(PVOID pTrieTree, PVOID pool, PVOID data);

//	销毁NODE pool
//		这里要做的事情是：
//		销毁释放整个 pTrieTree->poolNode
typedef PVOID (*ZooTypeFunc_TrieTree_DestroyNode)(PVOID pTrieTree, PVOID *pool);

//	当前函数用来在获取指定规则的时候，判断多层目录有多个规则的时候用哪个规则
//	比如，树里有
//	D:\\1\\2\\3\\4.c
//	D:\\1\\2\\3\\
//	D:\\1\\2\\
//	D:\\1\\
//	这样三条规则，当我用 TrieTree_GetSpecifyRuleCmp 获取规则时
//	如果我的Key 是： D:\\1\\2\\3\\4.c
//	那么调用下来的时候，就会触发当前函数 TrieTree_GetSpecifyRuleCmp
//	第一次：p 指向 D:\\1\\ 的规则，如果这个规则没有命中我们的 key，那就继续
//	第二次：p 指向 D:\\1\\2\\ 的规则
//	第三次：p 指向 D:\\1\\2\\3\\ 的规则
//	第四次命中：p 指向 D:\\1\\2\\3\\4.c 的规则
typedef ULONG (*ZooTypeFunc_TrieTree_GetSpecifyRuleCmp)(PVOID pThis, WCHAR *wsKey, PVOID pParam, PVOID pData);

//	内存设置函数，memset
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


//	TrieTree 初始化
//		原则上说必须先初始化然后再用，
//			函数列表中，有些函数是必须要用到的
//			plw 是用来做规则分段的标识数组，最多支持10个，
//				在目录的角度上就是 L'\\' 和 L'/'
ULONG TrieTree_Init(PVOID *pTrie, ULONG uDataSize, PTRIE_TREE_MEMORY_FUNCTION pFun, TRIE_INDEX* plw, ULONG uLwCount);


//	TrieTree 销毁树
ULONG TrieTree_Destroy(PVOID *pTrie);


//	TrieTree 插入数据
//		根据一个Key 把 data 插入到 TrieTree 里面
ULONG TrieTree_Insert(PVOID pTrie, WCHAR *wsKey, PVOID data);


//	TrieTree 修改数据
//		精确修改 TrieTree 中指定 Key 的数据，如果Key不存在，则放弃，这里是精确修改
//		如果 data 的值为NULL 的话，那么这里就和删除数据没有区别了
ULONG TrieTree_Revise(PVOID pTrie, WCHAR *wsKey, PVOID data);


//	TrieTree 查找数据
//		根据wsKey 精确查找元素，找到就把信息返回来，找不到就失败
ULONG TrieTree_Search(PVOID pTrie, WCHAR *wsKey, PVOID *pdata);


//	TrieTree 删除数据
//		删除的时候根本不需要把节点都删了，也不差那么几个字节，只要把数据区抹掉就好了
//		然后再找到这里的时候，当前Key 就没有数据了
ULONG TrieTree_Delete(PVOID pTrie, WCHAR *wsKey);


//	TrieTree 查找数据
//		这个查找是个模糊查找，根据规则一段一段查找，目前的规则是 \ 和 / 内置的
//		只要找到当前Key 所包含的Key ，并且规则分段方式相同，则认为找到数据
//	比如：
//		TrieTree 内部有一条Key 为 D:\\1\\
//		要找的Key 为 D:\\1\\2\\3.c
//		根据要求，因为 D:\\1\\ 有数据，所以我们寻找 D:\\1\\2\\3.c 的时候，遇到了 D:\\1\\ 就会直接返回数据
ULONG TrieTree_GetRule(PVOID pTrie, WCHAR *wsKey, PVOID *pv);


//	TrieTree 查找数据
//		这个查找也是个模糊查找，但是这里支持的是外部提供匹配函数来判断查找结果
//		所以这里很可能支持多个结果
//		具体情况看 TrieTree_GetSpecifyRuleCmpFunc 的定义
ULONG TrieTree_GetSpecifyRuleCmp(PVOID pTrie, WCHAR *wsKey, PVOID pParam);


//	TrieTree 查找数据
//		这个查找是个模糊查找，根据规则一段一段查找，目前的规则是 \ 和 / 内置的
//		只要找到当前Key 所包含的Key ，并且规则分段方式相同，则认为找到数据
//		从右侧开始找
ULONG TrieTree_GetRule_Right(PVOID pTrie, WCHAR *wsKey, PVOID *pv);


//	TrieTree 查找数据
//		这个查找也是个模糊查找，但是这里支持的是外部提供匹配函数来判断查找结果
//		所以这里很可能支持多个结果
//		具体情况看 TrieTree_GetSpecifyRuleCmpFunc 的定义
//		从右侧开始找
ULONG TrieTree_GetSpecifyRuleCmp_Right(PVOID pTrie, WCHAR *wsKey, PVOID pParam);


//	TrieTree 获取正在使用的node个数
ULONG TrieTree_GetNodeOnUseCount(PVOID pTrie);


//	TrieTree 获取申请node次数
ULONG TrieTree_GetAllocNodeCount(PVOID pTrie);


//	TrieTree 获取正在使用的Data个数
ULONG TrieTree_GetDataOnUseCount(PVOID pTrie);


//	TrieTree 获取申请Data次数
ULONG TrieTree_GetAllocDataCount(PVOID pTrie);


//	TrieTree 获取节点大小
ULONG TrieTree_GetNodeSize(PVOID pTrie);


//	TrieTree 获取Data大小
ULONG TrieTree_GetDataSize(PVOID pTrie);