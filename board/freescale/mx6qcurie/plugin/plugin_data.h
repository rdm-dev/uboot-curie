#ifndef _PLUGIN_DATA_H_
#define _PLUGIN_DATA_H_

#include <stdint.h>
#include "crc32.h"

// The total size of the plugin data must be less than this value
#define PLUGIN_DATA_SIZE_MAX    1024

// constant in header
#define PLUGIN_DATA_MAGIC       0x47554C50 // 'P', 'L', 'U', 'G'
#define PLUGIN_DATA_VER1        0x00000001 // version 1

// segment tags
enum {
	PLUGIN_DATA_TAG_RESERVED = 0,
	PLUGIN_DATA_TAG_DDR_PARAM = 13,
};

enum {
	PLUGIN_DATA_E_OK = 0,
	PLUGIN_DATA_E_MAGIC, // bad magic
	PLUGIN_DATA_E_VERSION, // unsupported version
	PLUGIN_DATA_E_LENGTH, // length too big
	PLUGIN_DATA_E_CHECKSUM, // bad checksum
	PLUGIN_DATA_E_END, // end of segment
	PLUGIN_DATA_E_INCOMPLETED, // segment data incompleted
};

// plugin data header
typedef struct tag_plugin_data_hdr_t
{
	uint32_t m_magic; // magic value
	uint32_t m_version; // version
	uint32_t m_length; // total length of content, aligned to 32bit word
	uint32_t m_checksum; // crc32 of all data, with header.checksum = 0
} __attribute__((packed)) plugin_data_hdr_t;

// plugin data segment
typedef struct tag_plugin_data_tlv_t
{
	uint16_t m_tag; // value tag
	uint16_t m_length; // length of value, aligned to 32bit word
	uint32_t m_value[0]; // value
} __attribute__((packed)) plugin_data_tlv_t;

// value structure for ddr calibration parameters
typedef struct tag_plugin_data_value_ddr_param_t
{
	struct {
		uint32_t m_mpdgctrl0;
		uint32_t m_mpdgctrl1;
		uint32_t m_mprddlctl;
		uint32_t m_mpwrdlctl;
	} __attribute__((packed)) m_phy0, m_phy1;
} __attribute__((packed)) plugin_data_value_ddr_param_t;

// iterator to access all the segments
typedef struct tag_plugin_data_itr_t
{
	plugin_data_hdr_t *m_hdr; // pointer to data header
	plugin_data_tlv_t *m_seg; // pointer to current segment, NULL if meet the end
} plugin_data_itr_t;

// initialize the iterator
int plugin_data_itr_init(plugin_data_itr_t *itr, void *data);

// move to next segment, return 0 if success.
int plugin_data_itr_next(plugin_data_itr_t *itr);

// total size of plugin data
#define plugin_data_size(hdr)    (sizeof(plugin_data_hdr_t) + (hdr)->m_length)


#endif

