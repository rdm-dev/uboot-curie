#include "plugin_data.h"
#include <stdio.h>

char data[PLUGIN_DATA_SIZE_MAX];

int main(int argc, char *argv)
{
	int total = 0;
	plugin_data_hdr_t *hdr;
	plugin_data_tlv_t *seg;
	plugin_data_value_ddr_param_t *ddr_param;
	FILE *fp;

	// header
	hdr = (plugin_data_hdr_t *) data;
	hdr->m_magic = PLUGIN_DATA_MAGIC;
	hdr->m_version = PLUGIN_DATA_VER1;
	hdr->m_length = sizeof(plugin_data_tlv_t) + sizeof(plugin_data_value_ddr_param_t);
	hdr->m_checksum = 0;

	// ddr param
	seg = (plugin_data_tlv_t *) (data + sizeof(plugin_data_hdr_t));
	seg->m_tag = PLUGIN_DATA_TAG_DDR_PARAM;
	seg->m_length = sizeof(plugin_data_value_ddr_param_t);
	// value
	ddr_param = (plugin_data_value_ddr_param_t *) seg->m_value;
	ddr_param->m_phy0.m_mpdgctrl0 = 0x03200338;
	ddr_param->m_phy0.m_mpdgctrl1 = 0x0328031C;
	ddr_param->m_phy0.m_mprddlctl = 0x3E2C3236;
	ddr_param->m_phy0.m_mpwrdlctl = 0x3A3E423E;
	ddr_param->m_phy1.m_mpdgctrl0 = 0x03280338;
	ddr_param->m_phy1.m_mpdgctrl1 = 0x03240268;
	ddr_param->m_phy1.m_mprddlctl = 0x36323242;
	ddr_param->m_phy1.m_mpwrdlctl = 0x46384C42;

	// generate crc
	total = sizeof(plugin_data_hdr_t) + hdr->m_length;
	hdr->m_checksum = crc32(0, hdr, total);

	printf("Plugin Data Generated.\n");

	// write plugin data
	fp = fopen("plugin_data.bin", "wb");
	if(!fp)
	{
		printf("Fail to open file to write.\n");
		return 1;
	};
	fwrite(data, total, 1, fp);
	fclose(fp);

	printf("Plugin Data Saved.\n");

	printf("Bye! Have a nice day!\n");

	return 0;
}

