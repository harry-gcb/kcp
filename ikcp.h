//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//  
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#ifndef __IKCP_H__
#define __IKCP_H__

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>


//=====================================================================
// 32BIT INTEGER DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#endif
#endif


//=====================================================================
// Integer Definition
//=====================================================================
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif


//=====================================================================
// QUEUE DEFINITION                                                  
//=====================================================================
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__

// 在 KCP 中仅有 2 种队列结构:
// 一种用于 acklist , 它以简单的指数增长的方式重新分配数组大小
// 另一种用于发送与接受队列和缓冲区 (snd_queue/rcv_queue/snd_buf/rcv_buf) 的队列 IQUEUEHEAD 结构
struct IQUEUEHEAD {
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;


//---------------------------------------------------------------------
// queue init                                                         
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


//---------------------------------------------------------------------
// queue operation                     
//---------------------------------------------------------------------
#define IQUEUE_ADD(node, head) ( \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

#define IQUEUE_ADD_TAIL(node, head) ( \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

#define IQUEUE_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) do { \
	IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0)

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init		IQUEUE_INIT
#define iqueue_entry	IQUEUE_ENTRY
#define iqueue_add		IQUEUE_ADD
#define iqueue_add_tail	IQUEUE_ADD_TAIL
#define iqueue_del		IQUEUE_DEL
#define iqueue_del_init	IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

#define iqueue_foreach_entry(pos, head) \
	for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )
	

#define __iqueue_splice(list, head) do {	\
		iqueue_head *first = (list)->next, *last = (list)->prev; \
		iqueue_head *at = (head)->next; \
		(first)->prev = (head), (head)->next = (first);		\
		(last)->next = (at), (at)->prev = (last); }	while (0)

#define iqueue_splice(list, head) do { \
	if (!iqueue_is_empty(list)) __iqueue_splice(list, head); } while (0)

#define iqueue_splice_init(list, head) do {	\
	iqueue_splice(list, head);	iqueue_init(list); } while (0)


#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

#endif


//---------------------------------------------------------------------
// BYTE ORDER & ALIGNMENT
//---------------------------------------------------------------------
#ifndef IWORDS_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #define IWORDS_BIG_ENDIAN  0
    #endif
#endif

#ifndef IWORDS_MUST_ALIGN
	#if defined(__i386__) || defined(__i386) || defined(_i386_)
		#define IWORDS_MUST_ALIGN 0
	#elif defined(_M_IX86) || defined(_X86_) || defined(__x86_64__)
		#define IWORDS_MUST_ALIGN 0
	#elif defined(__amd64) || defined(__amd64__)
		#define IWORDS_MUST_ALIGN 0
	#else
		#define IWORDS_MUST_ALIGN 1
	#endif
#endif


//=====================================================================
// SEGMENT kcp分段结构体
//=====================================================================

/* 数据fragment结构
0               4   5   6       8 (BYTE)
+---------------+---+---+-------+
|     conv      |cmd|frg|  wnd  |
+---------------+---+---+-------+   8
|     ts        |     sn        |
+---------------+---------------+  16
|     una       |     len       |
+---------------+---------------+  24
|                               |
|        DATA (optional)        |
|                               |
+-------------------------------+

conv 4 字节: 连接标识, 前面已经讨论过了.
cmd 1 字节: Command.
frg 1 字节: 分片数量. 表示随后还有多少个报文属于同一个包.
wnd 2 字节: 发送方剩余接收窗口的大小.
ts 4 字节: 时间戳.
sn 4 字节: 报文编号.
una 4 字节: 发送方的接收缓冲区中最小还未收到的报文段的编号. 也就是说, 编号比它小的报文段都已全部接收.
len 4 字节: 数据段长度.
data: 数据段. 只有数据报文会有这个字段.
*/
// IKCPSEG 结构仅为内存状态，仅有部分字段会编码到传输协议中
struct IKCPSEG
{
	struct IQUEUEHEAD node; // 双向链表定义的队列 用于发送和接受队列的缓冲
	IUINT32 conv;			// conversation ，会话序列号：接收到的数据包与发送的一致才接收此数据包
	IUINT32 cmd;			// command，指令类型：代表这个segment的类型
	IUINT32 frg;			// 分片编号，表示倒数第几个分片。
	IUINT32 wnd;			// 本地窗口大小，给接收端做流控的
	IUINT32 ts;				// 当前时间戳
	IUINT32 sn;				// 分配sn序列号
	IUINT32 una;			// 累计确认数据编号，发送端期望收到的数据包，代表编号前面的所有报都收到了的标志
	IUINT32 len;			// 数据长度
	IUINT32 resendts;		// 重传时间戳，超过这个时间重发这个包
	IUINT32 rto;			// 该报文的 RTO，重传的超时时间 (retransmission timeout)
	IUINT32 fastack;		// ACK 失序次数. 也就是 KCP Readme 中所说的 “跳过” 次数.
	IUINT32 xmit;			// 该报文已经发送的次数
	char data[1];			// 数据段
};


//---------------------------------------------------------------------
// IKCPCB
//---------------------------------------------------------------------
// IKCPCB 结构存储了 KCP 协议的所有上下文，通过创建对端的两个 IKCPCB 对象进行协议通讯
struct IKCPCB
{
	IUINT32 conv; 		// 会话id
	IUINT32 mtu;  		// 最大传输单元
	IUINT32 mss;  		// 最大分片大小, mtu-24
	IUINT32 state; 		// 连接状态(0xFFFFFFFF表示断开连接)
	IUINT32 snd_una;	// 第一个未确认的包
	IUINT32 snd_nxt;	// 待发送包的序号 
	IUINT32 rcv_nxt;	// 待接受包的序号，待接收消息尾序号(rcv_nxt+rcv_wnd)
	IUINT32 ts_recent;	// 
	IUINT32 ts_lastack;	//
	IUINT32 ssthresh;	// 拥堵窗口的阈值，用来控制增长速度，满启动阈值
	IINT32 rx_rttval;	// 平滑网络抖动时间
	IINT32 rx_srtt;		// 平滑往返时间
	IINT32 rx_rto;		//  Retransmission TimeOut(RTO), 超时重传时间.
	IINT32 rx_minrto;
	IUINT32 snd_wnd, rcv_wnd;
	IUINT32 rmt_wnd;	// 对端剩余接收窗口的大小
	IUINT32 cwnd, probe;
	IUINT32 current, interval, ts_flush, xmit;
	IUINT32 nrcv_buf;	// 记录接收消息缓冲区长度，rcv_buf
	IUINT32 nsnd_buf;	// 记录发送消息缓冲区长度，snd_buf
	IUINT32 nrcv_que;	// 记录接收队列长度，rcv_queue
	IUINT32 nsnd_que;	// 记录发送队列长度，snd_queue
	IUINT32 nodelay, updated;
	IUINT32 ts_probe, probe_wait;	// 确定何时需要发送窗口询问报文.
	IUINT32 dead_link, incr;
	struct IQUEUEHEAD snd_queue;	// 发送消息队列
	struct IQUEUEHEAD rcv_queue;	// 接收消息队列
	struct IQUEUEHEAD snd_buf;		// 发送消息缓存数据
	struct IQUEUEHEAD rcv_buf;		// 接受消息缓存数据
	IUINT32 *acklist;	// 待发送的ack列表
	IUINT32 ackcount;
	IUINT32 ackblock;
	void *user;
	char *buffer;		// flush 时用到的临时缓冲区.
	int fastresend;
	int fastlimit;
	int nocwnd;
	int stream;	// 是否开启流模式, 开启后可能会合并包, 默认不开启
	int logmask;
	int (*output)(const char *buf, int len, struct IKCPCB *kcp, void *user);
	void (*writelog)(const char *log, struct IKCPCB *kcp, void *user);
};

// 一个 ikcpcb 实例代表一个 KCP 连接
typedef struct IKCPCB ikcpcb;

#define IKCP_LOG_OUTPUT			1
#define IKCP_LOG_INPUT			2
#define IKCP_LOG_SEND			4
#define IKCP_LOG_RECV			8
#define IKCP_LOG_IN_DATA		16
#define IKCP_LOG_IN_ACK			32
#define IKCP_LOG_IN_PROBE		64
#define IKCP_LOG_IN_WINS		128
#define IKCP_LOG_OUT_DATA		256
#define IKCP_LOG_OUT_ACK		512
#define IKCP_LOG_OUT_PROBE		1024
#define IKCP_LOG_OUT_WINS		2048

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// interface
//---------------------------------------------------------------------

// create a new kcp control object, 'conv' must equal in two endpoint
// from the same connection. 'user' will be passed to the output callback
// output callback can be setup like this: 'kcp->output = my_udp_output'

// ikcp_create 创建一个 KCP 实例. 传入的 conv 参数标识这个 KCP 连接, 
// 也就是说, 这个连接发出去的每个报文段都会带上 conv, 它也只会接收 conv 与之相等的报文段. 
// 通信的双方必须先协商一对相同的 conv. KCP 本身不提供任何握手机制, 协商 conv 交给使用者自行实现, 
// 比如说通过已有的 TCP 连接协商.
ikcpcb* ikcp_create(IUINT32 conv, void *user);
// release kcp control object
// 释放一个 KCP 实例
void ikcp_release(ikcpcb *kcp); // 释放一个 KCP 实例

// set output callback, which will be invoked by kcp
// KCP 是纯算法实现的, 不负责下层协议收发, 内部没有任何系统调用, 连时钟都要外部传进来. 因此我们需要:
// 1. 	调用 ikcp_setoutput 设置下层协议输出函数. 每当 KCP 需要发送数据时, 都会回调这个输出函数. 
// 		例如下层协议是 UDP 时, 就在输出回调中调用 sendto 将数据发送给对方. 
// 		输出回调的 user 参数等于 ikcp_create 传入的 user 参数.
// 2.	当下层协议数据到达时, 调用 ikcp_input 将数据传给 KCP.
// 3.	以一定的频率调用 ikcp_update 以驱动 KCP 的时钟. current 表示当前时间, 单位为毫秒.
void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len, 
	ikcpcb *kcp, void *user)); // 设置下层协议输出回调函数

// user/upper level recv: returns size, returns below zero for EAGAIN
// ikcp_recv 从接收队列中读取数据;
int ikcp_recv(ikcpcb *kcp, char *buffer, int len);

// user/upper level send, returns below zero for error
// ikcp_send 将数据放在发送队列中等待发送;
int ikcp_send(ikcpcb *kcp, const char *buffer, int len);

// update state (call it repeatedly, every 10ms-100ms), or you can ask 
// ikcp_check when to call it again (without ikcp_input/_send calling).
// 'current' - current timestamp in millisec. 
void ikcp_update(ikcpcb *kcp, IUINT32 current); // 时钟更新

// Determine when should you invoke ikcp_update:
// returns when you should invoke ikcp_update in millisec, if there 
// is no ikcp_input/_send calling. you can call ikcp_update in that
// time, instead of call update repeatly.
// Important to reduce unnacessary ikcp_update invoking. use it to 
// schedule ikcp_update (eg. implementing an epoll-like mechanism, 
// or optimize ikcp_update when handling massive kcp connections)
IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current);

// when you received a low level packet (eg. UDP packet), call it
// ikcp_input 读取下层协议输入数据, 解析报文段; 
// 如果是数据, 就将数据放入接收缓冲区; 
// 如果是 ACK, 就在发送缓冲区中标记对应的报文段为已送达;
int ikcp_input(ikcpcb *kcp, const char *data, long size);

// flush pending data
// ikcp_flush 调用输出回调将发送缓冲区中的数据发送出去.
void ikcp_flush(ikcpcb *kcp);

// check the size of next message in the recv queue
int ikcp_peeksize(const ikcpcb *kcp);

// change MTU size, default is 1400
// 最大传输单元:
// KCP 不负责探测 MTU，默认值为 1400 字节，可以使用 ikcp_setmtu 来设置该值。该值将会影响数据包归并及分片时候的最大传输单元。较小的 MTU 会影响路由转优先级。
int ikcp_setmtu(ikcpcb *kcp, int mtu);

// set maximum window size: sndwnd=32, rcvwnd=32 by default
// 最大窗口选项:
// 发送窗口大小 sndwnd 必须大于 0，接收窗口大小 rcvwnd 必须大于 128，单位为包，而非字节。
int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd);

// get how many packet is waiting to be sent
int ikcp_waitsnd(const ikcpcb *kcp);

// fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
// nodelay: 0:disable(default), 1:enable
// interval: internal update timer interval in millisec, default is 100ms 
// resend: 0:disable fast resend(default), 1:enable fast resend
// nc: 0:normal congestion control(default), 1:disable congestion control
// 工作模式选项:
// nodelay :是否启用 nodelay模式，0不启用；1启用
// interval :协议内部工作的 interval，单位毫秒，比如 10ms或者 20ms
// resend :快速重传模式，默认0关闭，可以设置2（2次ACK跨越将会直接重传）
// nc :是否关闭流控，默认是0代表不关闭，1代表关闭
// 普通模式: ikcp_nodelay(kcp, 0, 40, 0, 0);
// 极速模式: ikcp_nodelay(kcp, 1, 10, 2, 1);
int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc);


void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...);

// setup allocator
void ikcp_allocator(void* (*new_malloc)(size_t), void (*new_free)(void*));

// read conv
IUINT32 ikcp_getconv(const void *ptr);


#ifdef __cplusplus
}
#endif

#endif


