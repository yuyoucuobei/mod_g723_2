/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthm@freeswitch.org>
 * Brian K. West <brian@freeswitch.org>
 *
 * The g723.1 codec itself is not distributed with this module.
 *
 * mod_g723_1.c -- G723.1 Codec Module
 *
 */

#include "switch.h"

#ifndef G723_PASSTHROUGH
#include "g723/g723.h"

#define TYPE_HIGH 0x0
#define TYPE_LOW 0x1
#define TYPE_SILENCE 0x2
#define TYPE_DONTSEND 0x3
#define TYPE_MASK 0x3

int   PackedFrameSize[2] = {
   24 ,
   20
   } ;

Flag    UseHp = False ;
Flag    UsePf = False ;
Flag    UseVx = False ;
Flag    UsePr = False ;

char  SignOn[] = "ITU-T G.723.1 ANSI C FIXED POINT CODEC Ver 5.2 (Release 2, June 2006)\n" ;

enum Crate WrkRate = Rate63;

enum Wmode WrkMode = Cod;

#endif

SWITCH_MODULE_LOAD_FUNCTION(mod_g723_1_load);
SWITCH_MODULE_DEFINITION(mod_g723_1, mod_g723_1_load, NULL, NULL);

#ifndef G723_PASSTHROUGH
struct g723_context {
	Word16 cod_float_buf[Frame];
	Word16 dec_float_buf[Frame];
};
#endif

static switch_status_t switch_g723_init(switch_codec_t *codec, switch_codec_flag_t flags, const switch_codec_settings_t *codec_settings)
{
#ifdef G723_PASSTHROUGH
	codec->flags |= SWITCH_CODEC_FLAG_PASSTHROUGH;
	return SWITCH_STATUS_FALSE;
#else
	struct g723_context *context = NULL;
	int encoding, decoding;

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_init(flags=%d)\n", flags);

	encoding = (flags & SWITCH_CODEC_FLAG_ENCODE);
	decoding = (flags & SWITCH_CODEC_FLAG_DECODE);

	if (!(encoding || decoding) || (!(context = switch_core_alloc(codec->memory_pool, sizeof(struct g723_context))))) 
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_init, return SWITCH_STATUS_FALSE\n");
		return SWITCH_STATUS_FALSE;
	} 
	else 
	{
		if (encoding) 
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_init, encoding\n");
			Init_Coder();
			if (UseVx) 
			{
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_init, UseVx\n");
				Init_Vad();
				Init_Cod_Cng();
			}
		}

		if (decoding) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_init, decoding\n");
			Init_Decod();
			Init_Dec_Cng();
		}

		codec->private_info = context;

		return SWITCH_STATUS_SUCCESS;
	}
#endif
}

static switch_status_t switch_g723_destroy(switch_codec_t *codec)
{
#ifndef G723_PASSTHROUGH
	codec->private_info = NULL;
#endif
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g723_encode(switch_codec_t *codec,
										  switch_codec_t *other_codec,
										  void *decoded_data,
										  uint32_t decoded_data_len,
										  uint32_t decoded_rate, void *encoded_data, uint32_t *encoded_data_len, uint32_t *encoded_rate,
										  unsigned int *flag)
{
#ifdef G723_PASSTHROUGH
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "This codec is only usable in passthrough mode!\n");
	return SWITCH_STATUS_FALSE;
#else
	struct g723_context *context = codec->private_info;
	int16_t *decoded_slin_buf = (int16_t *) decoded_data;
	char *ebuf = (char *) encoded_data;
	int x;
	WrkMode = Cod ;

	// switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, 
	// 	"switch_g723_encode(decoded_data_len=%d, decoded_rate=%d, encoded_data_len=%d, encoded_rate=%d, flag=%d)\n",
	//  	decoded_data_len, decoded_rate, *encoded_data_len, *encoded_rate, *flag);

	if (!context) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_encode, !context\n");
		return SWITCH_STATUS_FALSE;
	}

	for (x = 0; x < Frame; x++) {
		context->cod_float_buf[x] = decoded_slin_buf[x];
	}

	Coder((Word16 *) context->cod_float_buf, ebuf);
	*encoded_data_len = codec->implementation->encoded_bytes_per_packet;
	*encoded_rate = decoded_rate;

	// switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, 
	// 	"switch_g723_encode(encoded_data_len=%d, encoded_rate=%d)\n", 
	// 	*encoded_data_len, *encoded_rate);

	return SWITCH_STATUS_SUCCESS;
#endif
}

static switch_status_t switch_g723_decode(switch_codec_t *codec,
										  switch_codec_t *other_codec,
										  void *encoded_data,
										  uint32_t encoded_data_len,
										  uint32_t encoded_rate, void *decoded_data, uint32_t *decoded_data_len, uint32_t *decoded_rate,
										  unsigned int *flag)
{
#ifdef G723_PASSTHROUGH
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "This codec is only usable in passthrough mode!\n");
	return SWITCH_STATUS_FALSE;
#else
	struct g723_context *context = codec->private_info;
	int x;
	int16_t *to_slin_buf = (int16_t *)decoded_data;
	WrkMode = Dec ;

	// switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_decode(flag=%d)\n", *flag);

	if (!context) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "switch_g723_decode, !context\n");
		return SWITCH_STATUS_FALSE;
	}

	Decod((Word16 *) context->dec_float_buf, (char *) encoded_data, 0);

	for (x = 0; x < Frame; x++) {
		to_slin_buf[x] = context->dec_float_buf[x];
	}
	*decoded_data_len = codec->implementation->decoded_bytes_per_packet;
	*decoded_rate = encoded_rate;

	return SWITCH_STATUS_SUCCESS;
#endif
}

SWITCH_MODULE_LOAD_FUNCTION(mod_g723_1_load)
{
	switch_codec_interface_t *codec_interface;
	int ompf = 30000, ospf = 240, obpf = 480, oebpf = 24, count = 0;
	int mpf = ompf, spf = ospf, bpf = obpf, ebpf = oebpf;

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	SWITCH_ADD_CODEC(codec_interface, "G.723.1 6.3k");

	for (count = 0; count < 4; count++) {
		switch_core_codec_add_implementation(pool, codec_interface,
											 SWITCH_CODEC_TYPE_AUDIO, 4, "G723", NULL, 8000, 8000, 6300,
											 mpf, spf, bpf, ebpf, 1, count, switch_g723_init, switch_g723_encode, switch_g723_decode, switch_g723_destroy);
		mpf += ompf;
		spf += ospf;
		bpf += obpf;
		ebpf += oebpf;


	}
	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */
