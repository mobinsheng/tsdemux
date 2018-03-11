#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_tsdemux.h"

/*
 * 解析packet的自适应字段、payload
 */
int
ts__packet_decode(ts_packet_t *packet)
{
	uint16_t progid;
	int n;
	
	progid = 0;
	if(packet->pid == PID_NULL)
	{
		return 0;
	}

    // 判断pid对应的描述信息是否存在，如果不存在那么创建它
	if(NULL == (packet->pidinfo = ts_stream_pid_get(packet->stream, packet->pid)))
	{
		packet->pidinfo = ts__stream_pid_add(packet->stream, packet->pid);
	}
    // 可见，表示已经存在了
	packet->pidinfo->seen = 1;

    // 如果是协议预定义的pid（PAT、CAT之类的），那么开始解析
	if(PT_SECTIONS == packet->pidinfo->pidtype)
	{
		while(packet->plofs < packet->payloadlen)
		{
			if(1 != ts__table_decode(packet, progid))
			{
				break;
			}
		}
		return 0;
	}
	/* Elementary stream? */
	return 0;
}
