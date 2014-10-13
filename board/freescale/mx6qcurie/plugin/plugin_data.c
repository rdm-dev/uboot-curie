#include "plugin_data.h"

static int plugin_data_check_header(plugin_data_hdr_t *hdr)
{
	uint32_t checksum = hdr->m_checksum;

	// magic
	if(hdr->m_magic != PLUGIN_DATA_MAGIC)
		return PLUGIN_DATA_E_MAGIC;

	// version, currently only ver1 is supported
	if(hdr->m_version != PLUGIN_DATA_VER1)
		return PLUGIN_DATA_E_VERSION;

	// check the total length
	if(hdr->m_length + sizeof(plugin_data_hdr_t) > PLUGIN_DATA_SIZE_MAX)
		return PLUGIN_DATA_E_LENGTH;

	// backup checksum
	checksum = hdr->m_checksum;
	// set checksum in header to zero
	hdr->m_checksum = 0;
	// calculate the checksum again and compare it to the backup one
	if(checksum != crc32(0, hdr, hdr->m_length + sizeof(plugin_data_hdr_t)))
		return PLUGIN_DATA_E_CHECKSUM;

	return PLUGIN_DATA_E_OK;
}

int plugin_data_itr_init(plugin_data_itr_t *itr, void *data)
{
	int ret;

	// bind header
	itr->m_hdr = (plugin_data_hdr_t *) data;

	// verify header
	if( (ret = plugin_data_check_header(itr->m_hdr)) )
		return ret;

	// move to the first segment
	if(itr->m_hdr->m_length >= sizeof(plugin_data_tlv_t))
	{
		itr->m_seg = (plugin_data_tlv_t *) ( ((char *) itr->m_hdr) + sizeof(plugin_data_hdr_t) );
	}
	else
	{
		itr->m_seg = 0;
		return PLUGIN_DATA_E_END;
	};

	return PLUGIN_DATA_E_OK;
}

int plugin_data_itr_next(plugin_data_itr_t *itr)
{
	char *end = ((char *) itr->m_hdr) + sizeof(plugin_data_hdr_t) + itr->m_hdr->m_length;
	char *p = (char *) itr->m_seg;

	// already meet the end
	if(!itr->m_seg)
		return PLUGIN_DATA_E_END;

	// move to next data
	p += sizeof(plugin_data_tlv_t) + itr->m_seg->m_length;
	// check if meet the end of content
	if(p >= end)
	{
		itr->m_seg = 0;
		return PLUGIN_DATA_E_END;
	};

	// save current seg
	itr->m_seg = (plugin_data_tlv_t *) p;
	// check if the whole segment is inside the data
	if(p + sizeof(plugin_data_tlv_t) + itr->m_seg->m_length > end)
	{
		itr->m_seg = 0;
		return PLUGIN_DATA_E_INCOMPLETED;
	};
	return PLUGIN_DATA_E_OK;
}


