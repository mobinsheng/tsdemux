/* stream.c: basic stream and packet-handling */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

static void *
xalloc(size_t nbytes)
{
	void *p;
	
	if(NULL == (p = calloc(1, nbytes)))
	{
		fprintf(stderr, "Memory allocation error: failed to allocate %lu bytes\n", (unsigned long) nbytes);
		abort();
	}
	return p;
}

ts_stream_t *
ts_stream_create(const ts_options_t *opts)
{
	ts_stream_t *p;
	void *(*alloc)(size_t ptr);
	
	if(NULL == opts->allocmem)
	{
		alloc = xalloc;
	}
	else
	{
		alloc = opts->allocmem;
	}
	if(NULL == (p = (ts_stream_t *) alloc(sizeof(ts_stream_t))))
	{
		return NULL;
	}
	p->opts = opts;
	p->allocmem = alloc;
	if(NULL == opts->freemem)
	{
		p->freemem = free;
	}
	else
	{
		p->freemem = opts->freemem;
	}
	if(NULL == opts->reallocmem)
	{
		p->reallocmem = realloc;
	}
	else
	{
		p->reallocmem = opts->reallocmem;
	}
    /// NIT和TDT的PID设置为协议预定义的PID（最好设置为协议预定义的）
	p->nitpid = PID_DVB_NIT;
	p->tdtpid = PID_DVB_TDT;
	return p;
}

/*
 * 从文件中读取一个TS包，并解析
 */
int
ts_stream_read_packetf(ts_stream_t *stream, ts_packet_t *packet, FILE *src)
{
	size_t c, plen, bp, prepad;
	int n;
	uint8_t buf[192], *bufp;

    // 包的长度188
	plen = TS_PACKET_LENGTH;
	bufp = buf;
	prepad = 0;
    /// 这些可以不用理会，我们假设文件以标准的0x47开头
	if(stream->opts->timecode)
	{
		plen += sizeof(uint32_t);
		prepad += sizeof(uint32_t);
	}
	if(stream->opts->prepad)
	{
		for(c = 0; c < stream->opts->prepad && EOF != fgetc(src); c++);
		if(c < stream->opts->prepad)
		{
			return -1;
		}
	}

    // 解析数据
	if(0 == stream->opts->timecode && 1 == stream->opts->autosync)
	{
		bp = 0xFFFF;
		c = 0;

        // 最后一个packet
		if(stream->lastsync)
		{
			for(c = 0; c <= stream->lastsync; c++)
			{
				if(EOF == (n = fgetc(src)))
				{
					return -1;
				}
				if(0xFFFF == bp && TS_SYNC_BYTE == n)
				{
					bp = c;
				}
				bufp[c] = n;
			}
			if(TS_SYNC_BYTE == n)
			{
				/* Sync byte position matches last packet */
				bufp[0] = TS_SYNC_BYTE;
				plen--;
				bufp++;
			}
			else if(bp != 0xFFFF)
			{
				/* Sync byte occurred early */
				fprintf(stderr, "%s: Retraining; sync occurred at relative 0x%02x, expected at 0x%02x\n", stream->opts->filename, (uint8_t) bp, (uint8_t) stream->lastsync);
				if(bp)
				{
					/* Move everything from &bufp[bp] back to the start of the
					 * buffer and adjust plen
					 */
					memmove(bufp, &(bufp[bp]), stream->lastsync - bp);
					bufp += stream->lastsync - bp;
					plen -= stream->lastsync - bp;
				}
				stream->lastsync = bp;
			}
		}

        // 常规的packet，从文件中读取一个packet
        if(0xFFFF == bp)
		{
            // 寻找0x47
			for(; 0 == stream->opts->synclimit || c < stream->opts->synclimit; c++)
			{
				if(EOF == (n = fgetc(src)))
				{
					return -1;
				}
                // 遇到packet的开头sync字节
				else if(TS_SYNC_BYTE == n)
				{
					break;
				}
			}
            // 没读到0x47，出错！
			if(n != TS_SYNC_BYTE)
			{
				return -1;
			}
			if(stream->lastsync != c)
			{
				fprintf(stderr, "%s: skipped %lu bytes (autosync)\n", stream->opts->filename, (unsigned long) c);
			}
			stream->lastsync = c;
            // 保存0x47
			bufp[0] = n;
			bufp++;
			plen--;
		}
	}
    // 读取除了0x47之外的数据
	if(1 != fread(bufp, plen, 1, src))
	{
		return -1;
	}
	if(stream->opts->postpad)
	{
		for(c = 0; c < stream->opts->postpad && EOF != fgetc(src); c++);
		if(c < stream->opts->postpad)
			{
				return -1;
			}
	}
    // 开始解析这块内存
	return ts_stream_read_packet(stream, packet, buf, prepad);
}

int
ts_stream_read_packet(ts_stream_t *stream, ts_packet_t *packet, const uint8_t *bufp, size_t prepad)
{
	memset(packet, 0, sizeof(ts_packet_t));
	if(TS_PACKET_LENGTH + prepad > sizeof(packet->payload))
	{
		/* Packet too big for buffer */
		return -1;
	}
    // 一般来说，prepad都是0
	memcpy(packet->payload, bufp, TS_PACKET_LENGTH + prepad);
	packet->prepad = prepad;
	packet->payloadlen = TS_PACKET_LENGTH + prepad;
	packet->plstart = packet->plofs = prepad;
	bufp = &(packet->payload[prepad]);
	packet->sync = bufp[0];
    // 同步字节检验
	if(TS_SYNC_BYTE != packet->sync)
	{
		/* Invalid packet */
		fprintf(stderr, "%s: invalid sync byte at start of packet (expected 0x47, found 0x%02x)\n", stream->opts->filename, packet->sync);
		return -1;
	}
	packet->stream = stream;
    // 序号累加
	stream->seq++;
	if(stream->opts->timecode)
	{
		if(prepad != sizeof(uint32_t))
		{
			fprintf(stderr, "%s: Internal error: timecode option specified but prepad was not the correct size\n", stream->opts->filename);
			return -1;
		}
		packet->timecode = (packet->payload[0] << 24) | (packet->payload[1] << 16) | (packet->payload[2] << 8) | packet->payload[3];
	}
	bufp++; /* Skip sync byte */
	packet->plofs++;
    // 头部属性的解析
	packet->transerr = (bufp[0] & 0x80) >> 7;
	packet->unitstart = (bufp[0] & 0x40) >> 6;
	packet->priority = (bufp[0] & 0x20) >> 5;
	packet->pid = ((bufp[0] & 0x1f) << 8) | bufp[1];
	packet->sc = (bufp[2] & 0xc0) >> 6;
	packet->hasaf = (bufp[2] & 0x20) >> 5;
	packet->haspd = (bufp[2] & 0x10) >> 4;
	packet->continuity = bufp[2] & 0x0f;
	bufp += 3;
	packet->plofs += 3;
	packet->payloadlen = 184;
    // 解析剩下的自适应字段或者payload
	return ts__packet_decode(packet);
}
