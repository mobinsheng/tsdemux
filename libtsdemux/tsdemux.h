#ifndef TSDEMUX_H_
# define TSDEMUX_H_                    1

# include <stdio.h>
# include <inttypes.h>
#include <stdint.h>

/// PID的定义（TS协议预定义的，PMT的PID可以自定义）
# define PID_PAT                       0x0000 /// PAT节目关联表
# define PID_CAT                       0x0001 /// 条件接收表
# define PID_TSDT                      0x0002 /// 传输流描述表

/// 下面的PID与DVB相关，也是协议预定义的，一般比较少用
# define PID_DVB_NIT                   0x0010 /// 网络信息表
# define PID_DVB_SDT                   0x0011 /// 业务描述表
# define PID_DVB_EIT                   0x0012 /// 事件信息表
# define PID_DVB_RST                   0x0013 /// 运行状态表
# define PID_DVB_TDT                   0x0014 /// 时间日期表
# define PID_DVB_SYNC                  0x0015 ///
# define PID_DVB_INBAND                0x001c
# define PID_DVB_MEASUREMENT           0x001d
# define PID_DVB_DIT                   0x001e /// 间断信息表
# define PID_DVB_SIT                   0x001f /// 选择信息表
# define PID_NULL                      0x1fff
# define PID_UNSPEC                    0xffff

///ES（基本码流：音频、视频、字幕等）
# define ES_TYPE_MPEG1V                0x01 /// mpeg1视频
# define ES_TYPE_MPEG2V                0x02 /// mpeg2视频
# define ES_TYPE_MPEG1A                0x03 /// mpeg1音频
# define ES_TYPE_MPEG2A                0x04 ///mpeg2音频
# define ES_TYPE_PRIVATESECTS          0x05 /// 私有段
# define ES_TYPE_PRIVATEDATA           0x06 /// 私有数据
# define ES_TYPE_MHEG                  0x07
# define ES_TYPE_DSMCC                 0x08
# define ES_TYPE_AUXILIARY             0x09
# define ES_TYPE_DSMCC_ENCAP           0x0a
# define ES_TYPE_DSMCC_UN              0x0b
# define ES_TYPE_AAC                   0x0f /// AAC音频数据
# define ES_TYPE_MPEG4V                0x10 ///mpeg-4视频数据
# define ES_TYPE_LATM_AAC              0x11 /// AAC数据
# define ES_TYPE_MPEG4_GENERIC         0x12 /// mpeg-4数据（音频、视频、字幕？）
/* Unknown 0x13 */
# define ES_TYPE_DSMCC_DOWNLOAD        0x14
/* Unknown 0x15 */
/* Unknown 0x16 */
/* Unknown 0x17 */
/* Unknown 0x1a */
# define ES_TYPE_H264                  0x1b /// h264视频数据
 /// 下面也是一些音视频数据类型
# define ES_TYPE_DIGICIPHER2V          0x80
# define ES_TYPE_AC3                   0x81
# define ES_TYPE_DCA                   0x82
# define ES_TYPE_LPCM                  0x83
# define ES_TYPE_SDDS                  0x84
# define ES_TYPE_ATSC_PROGID           0x85
# define ES_TYPE_DTSHD                 0x86
# define ES_TYPE_EAC3                  0x87
# define ES_TYPE_DTS                   0x8a
# define ES_TYPE_DVB_SLICE             0x90
# define ES_TYPE_AC3B                  0x91
# define ES_TYPE_SUBTITLE              0x92
# define ES_TYPE_SDDSB                 0x94
# define ES_TYPE_MSCODEC               0xa0
# define ES_TYPE_DIRAC                 0xd1
# define ES_TYPE_VC1                   0xea

/// table ID（表格ID）
# define TID_PAT                       0x00 /// PAT
# define TID_CAT                       0x01 /// CAT
# define TID_PMT                       0x02 /// PMT
# define TID_DVB_NIT                   0x40 /// NIT
# define TID_DVB_ONIT                  0x41 /// ONIT

/// 描述标志
# define DESC_VIDEO                    0x02
# define DESC_AUDIO                    0x03
# define DESC_HIERARCHY                0x04
# define DESC_REGISTRATION             0x05
# define DESC_DSA                      0x06 /* data stream alignment */
# define DESC_TBG                      0x07 /* target background grid */
# define DESC_VIDEOWINDOW              0x08
# define DESC_CA                       0x09
# define DESC_LANGUAGE                 0x0a
# define DESC_CLOCK                    0x0b
# define DESC_MBU                      0x0c /* multiplex buffer utilisation */
# define DESC_COPYRIGHT                0x0d
# define DESC_MAXBITRATE               0x0e

# define PT_UNSPEC                     0
# define PT_SECTIONS                   1
# define PT_PES                        2
# define PT_DATA                       3
# define PT_NULL                       4

# define PST_UNSPEC                    0
# define PST_VIDEO                     1
# define PST_AUDIO                     2
# define PST_INTERACTIVE               3
# define PST_CC                        4
# define PST_IP                        5
# define PST_SI                        6
# define PST_NI                        7

typedef struct ts_stream_struct ts_stream_t;
typedef struct ts_options_struct ts_options_t;
typedef struct ts_rawpacket_struct ts_rawpacket_t;
typedef struct ts_adapt_struct ts_adapt_t;
typedef struct ts_packet_struct ts_packet_t;
typedef struct ts_pidinfo_struct ts_pidinfo_t;
typedef struct ts_pat_struct ts_pat_t;
typedef struct ts_pmt_struct ts_pmt_t;
typedef struct ts_table_struct ts_table_t;
typedef struct ts_streamtype_struct ts_streamtype_t;

/// 选项
struct ts_options_struct
{
	unsigned int timecode:1;
	unsigned int autosync:1;
	uint8_t prepad;
	uint8_t postpad;
	uint16_t synclimit;
	const char *progname;
	const char *filename;
	void *(*allocmem)(size_t nbytes);
	void (*freemem)(void *ptr);
	void *(*reallocmem)(void *ptr, size_t nbytes);
};

/* ts_stream_t: represents the entire transport stream */
/// 表示一个TS传输流
struct ts_stream_struct
{
	const ts_options_t *opts;
    uint64_t seq; /// 序列号
	size_t lastsync;
	/* The current program association table */
    ts_table_t *pat; /// PAT
	/* The list of all known tables */
    size_t ntables; /// 其他表格：CAT、PMT、NIT等
	ts_table_t **tables;
	/* The list of known PIDs */
    size_t npids; /// PID描述信息
	ts_pidinfo_t **pidinfo;
	/* Well-known PIDs */
    uint16_t nitpid; /// NIT的PID
    uint16_t tdtpid; /// TDT（时间日期表）的PID
	void *(*allocmem)(size_t nbytes);
	void (*freemem)(void *ptr);
	void *(*reallocmem)(void *ptr, size_t nbytes);
};

///
/*
 * PAT
 * PAT是一个TS流的有效标志
 * 只有找到PAT，TS流才能正确解码，它的作用就像SPS对应于H264
 * 一个PAT中描述了多个节目的信息（主要是节目的Program number和PMT的PID）
 * 一个节目和一个PMT是一一对应的，他们通过PMT的PID来联系
 * PMT中描述了多个打包码流（主要是PES的PID之类的）的信息
 * 一个PES就是打包过的ES码流，因为PES只是ES加了个头，因子不再仔细区分PES和ES
 */
struct ts_pat_struct
{
    /// 节目（多个）
	size_t nprogs;
	ts_table_t **progs; /* Reference the PMT of each program */
};

/*
 * PMT节目映射表
 */
struct ts_pmt_struct
{
    /// PCR，时钟信息的PID
	uint16_t pcrpid;
    /// 包含多少个基本码流
	size_t nes;
	ts_pidinfo_t **es; /* Reference the elementary streams for this program */
};

/*
 * PID描述信息
 */
struct ts_pidinfo_struct
{
	unsigned int seen:1;
	unsigned int defined:1;
    unsigned int pcr:1; /// PCR标志
    unsigned int pidtype; /// PID的类型
    unsigned int subtype; /// 子类型
	uint16_t pid;
	/* If it's a PES PID: */
	uint16_t pmtpid; /* PID of the programme this PES relates to */
    uint8_t stype;   /* Stream type from the PMT */ /// 流的类型
    uint8_t streamid; /// 流的ID
};

/*
 * 包头部的自适应字段
 */
struct ts_adapt_struct
{
	uint8_t len;
	unsigned int discontinuity:1;
	unsigned int random:1;
	unsigned int priority:1;
	unsigned int pcr:1;
	unsigned int opcr:1;
	unsigned int splicepoint:1;
	unsigned int privdata:1;
	unsigned int extensions:1;
};

/*
 * table的定义
 * 可以存放各种PAT、PMT、CAT等等
 */
struct ts_table_struct
{
	ts_table_t *prev, *next;
	uint16_t pid; /* The PID this table was last carried in */
    unsigned int expected:1; /* This table hasn't been defined yet */ /// 这个表格是否为协议预定义的
    size_t occurrences; /// 这个不是很清楚
	uint16_t progid; /* The progid this table was associated with, via the PAT */
	uint8_t tableid;
	unsigned int syntax:1;
	uint16_t seclen;
	uint16_t program;
	uint8_t version;
	unsigned int curnext:1;
	uint8_t section;
	uint8_t last;
	uint8_t *data;
	size_t datalen;
	uint32_t crc32;
	union
	{
		ts_pat_t pat;
		ts_pmt_t pmt;
	} d;
};

/*
 * TS包
 * 包的长度是188个字节，其中头部4个字节，payload字段184字节
 */
struct ts_packet_struct
{
	ts_stream_t *stream;
	uint32_t timecode;
	uint8_t sync;
	int transerr;
	int unitstart;
	int priority;
	uint16_t pid;
	int sc;
    int hasaf; /// 是否包含自适应字段
    int haspd; /// ？
	unsigned int continuity:1;
	ts_pidinfo_t *pidinfo;
	ts_adapt_t af; /* If hasaf is set */
	size_t ntables;
	ts_table_t *tables[8]; /* If this packet contains tables, pointers to them */
	ts_table_t *curtable;
	size_t payloadlen;
	size_t prepad; /* The offset of the packet header */
	size_t plofs; /* The (current) offset of the packet payload */
	size_t plstart; /* Start of the packet payload */ 
	uint8_t payload[192]; /* If haspd is set */
};

/*
 * 流的类型的描述信息
 */
struct ts_streamtype_struct
{
	uint8_t stype;
	uint8_t pidtype;
	uint8_t subtype;
	const char *name;
	const char *mime;
	const char *ext;
};

# ifdef __cplusplus
extern "C" {
# endif


    /// 创建一个TS流
	ts_stream_t *ts_stream_create(const ts_options_t *opts);
	
	/* Read a packet from a file and then decode it, dealing with padding
	 * based upon the options associated with the stream.
	 */
    /// 从文件中读取一个TS包，并解析
	int ts_stream_read_packetf(ts_stream_t *stream, ts_packet_t *dest, FILE *src);

	/* Process a pre-read packet. The packet must be 188 bytes long and start
	 * with a sync byte, unless the stream options indicate that a timecode
	 * is present, in which case the packet will be prepended with a 32-bit
	 * timecode value, resulting in a total of 192 bytes.
	 */
    /// 从一块内存中读取一个TS包，并解析
	int ts_stream_read_packet(ts_stream_t *stream, ts_packet_t *dest, const uint8_t *bufp, size_t prepad);
	
	/* Retrieve the table with the given table_id (and optional PID) */
    /// 根据PID、table id查找一个table
	ts_table_t *ts_stream_table_get(ts_stream_t *stream, uint8_t tableid, uint16_t pid);
	
	/* Retrieve the metadata for a particular PID */
    /// 根据PID查找PID的描述信息
	ts_pidinfo_t *ts_stream_pid_get(ts_stream_t *stream, uint16_t pid);

	/* Retrieve information about a given defined stream type */
    /// 根据流的类型查找描述信息
	const ts_streamtype_t *ts_typeinfo(uint8_t stype);
	
# ifdef __cplusplus
}
# endif

#endif /*!TSDEMUX_H_ */
