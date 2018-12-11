/*
 * oled.c
 *
 * Created: 2014/3/12 15:50:52
 *  Author: Airj
 */ 

#include "oled/oled.h"
#include "oled_driver.h"
#include "oled/oled_img.h"
#include <string.h>
#include <math.h>

typedef struct Rect
{
	uint8_t x,y,w,h;
} Rect_t;

Rect_t g_screen_left =
{
	.x = 0,
	.y = 0,
	.w = (MAX_Column>>1),
	.h = MAX_ROW,
};
Rect_t g_screen_right =
{
	.x = (MAX_Column>>1),
	.y = 0,
	.w = (MAX_Column>>1),
	.h = MAX_ROW,
};

Rect_t g_mode_tag =
{
	.x = 40,
	.y =0,
	.w = 64,
	.h = 16,
};

Rect_t g_hold_tag =
{
// 	.x = 194,
// 	.y = 3,
// 	.w = 32,
// 	.h = 10,
	//.x = 60,
	//.y =3,
	//.w = 32,
	//.h = 10,

	.x = 40,
	.y =0,
	.w = 68,
	.h = 16,
};
Rect_t g_real_time_pm25_tag =
{
	.x = 42,
	.y = 37,
	.w = 64,
	.h = 16,
};
Rect_t g_batery_tag =
{
	.x = 232,
	.y = 0,
	.w = 24,
	.h = 8,
};
Rect_t g_batery_fullscreen_tag =
{
	.x = 108,
	.y = 16,
	.w = 96,
	.h = 32,
};
const Rect_t g_real_time_pm25_num =
{
	.x = 110,
	.y = 26,
	.w = 18,
	.h = 30,
};
Rect_t g_real_time_pm25_unit =
{
	.x = 214,
	.y = 38,
	.w = 38,
	.h = 15,
};


Rect_t g_level_post =
{
	.x = 150,
	.y = 40,
	.w = 16,
	.h = 16,
};
Rect_t g_aqi_description_powersave =
{
	.x = 176,
	.y = 40,
	.w = 72,
	.h = 16,
};
Rect_t g_aqi_description_realtime =
{
	.x  = 120,
	.y  = 3,
	.w = 72,
	.h = 16,
};
Rect_t g_lowpowertips_tag1 =
{
	.x = 80,
	.y = 24,
	.w = 72,
	.h = 16,
};
Rect_t g_lowpowertips_tag2 =
{
	.x = 152,
	.y = 24,
	.w = 56,
	.h = 16,
};

uint8_t g_temp_for_program[180];

static void oled_set_pos(uint16_t x,uint16_t y)
{
	x = x>>1;
	y = YMAX-y-1;
	oled_wcmd(0xb0);
	oled_wcmd(y);
	oled_wcmd(x&0x0f);
	oled_wcmd(0x10|((x>>4)&0x07));
}

static void oled_draw_rectangle(Rect_t * pos, uint8_t data)
{
	uint16_t i,j;
	Assert((pos->x & 1 )!=0);
	Assert((pos->w & 1 )!=0);
	for (i=0; i<pos->h; i++)
	{
		oled_set_pos(pos->x,pos->y+i);
		for (j=0; j<pos->w; j+=2)
		{
			oled_wdat(data);
		}
	}
}

static inline void oled_fill(Rect_t * pos)
{
	oled_draw_rectangle(pos, 0xff);
}
static inline void oled_clear(Rect_t * pos)
{
	oled_draw_rectangle(pos, 0);
}
static inline void oled_clear_screen(void)
{
	Rect_t pos_screen =
	{
		.x = 16,
		.y = 0,
		.w = 240,
		.h = 64,
	};
	oled_clear(&pos_screen);
	//oled_clear(&g_screen_left);
	//oled_clear(&g_screen_right);
}
static void oled_getdataFromPgm(const uint8_t * pgm, uint8_t size)
{
	uint16_t i;
	memset(g_temp_for_program,0,sizeof(g_temp_for_program));
	for (i=0;i<min(180,size);i++)
	{
		g_temp_for_program[i] = pgm_read_byte(pgm+i);
	}
}

static void oled_show_map( uint8_t *map, const Rect_t * position, int8_t scale)
{
	uint16_t i, j;
	uint16_t x = position->x, y = position->y;
	uint16_t bitperline = ((((position->w -1)>>3)+1)<<3)>>scale;
	uint16_t mapindex;
	uint8_t mapmask;
	bool paint;
	uint8_t tmpbyte;
	
	Assert((position->x & 1 )==0);
	Assert((position->w & 1 )==0);
	for (i=0;i<position->h;i++)
	{
		oled_set_pos(x,y+i);
		tmpbyte = 0;
		for (j=0;j<position->w;j++)
		{
			mapindex = (((i>>scale)*bitperline)+(j>>scale))>>(3);
			mapmask = 0x80>>((j>>scale)&0x07);
			if (map[mapindex]&mapmask)
			{
				paint = true;
			}
			else
			{
				paint = false;
			}
			if ((x+j)&0x01)
			{
				if (paint)
				{
					tmpbyte |= 0x33;
				}
				oled_wdat(tmpbyte);
			}
			else
			{
				if (paint)
				{
					tmpbyte = 0xcc;
				}
				else
				{
					tmpbyte = 0;
				}
			}
			
		}
// 		if ((x+j)&0x01)
// 		{
// 			oled_wdat(tmpbyte);
// 		}
	}
	
}

static inline void oled_program_show_map(const uint8_t *map, const Rect_t * position, int8_t scale, uint8_t size)
{
	oled_getdataFromPgm(map, size);
	oled_show_map(g_temp_for_program, position, scale);
}
static void oled_shownote(const uint8_t *note, Rect_t *position, float xscale_bit,float yscale_bit)
{
	uint16_t i,j,k,index=0;
	uint8_t *tmpbytes = (uint8_t *)malloc((position->w>>1)*sizeof(uint8_t));
	uint8_t xtmp,ytmp;
	uint8_t m2sbytes[] = {0xcc,0x33};
	memset(tmpbytes,0,(position->w>>1)*sizeof(uint8_t));
	for (i=0;i<position->h;i++)
	{
		while(!(*(note+index)&0x80))
		{
			index++;
		}
		if (i+position->y<0)
		{
			continue;
		}
		ytmp = round(((*(note+index)&0x7f))*yscale_bit);
		while (ytmp<0)
		{
			index++;
			if(((*(note+index)&0x80)))
			{
				if (round(((*(note+index)&0x7f))*yscale_bit)>=0)
				ytmp = i;
			}
		}
		if (ytmp<i)
		{
			i--;
		}
		oled_set_pos(position->x,i+position->y);
		if (i == ytmp)
		{
			bool paint = false;
			index++;
			for (j=0; j<(position->w>>1); j++)
			{
				uint8_t tbyte = 0;
				for (k=0; k<2; k++)
				{
					if(!((*(note+index)&0x80)))
					{
						xtmp = round((*(note+index))*xscale_bit);
						while (xtmp<0)
						{
							index++;
							paint = !paint;
							if ((*(note+index+1)&0x80))
							{
								break;
							}
							xtmp = round((*(note+index))*xscale_bit);
						}
						if (((j<<1)+k)==xtmp)
						{
							if (!(*(note+index+1)&0x80)&&(xtmp == round((*(note+index+1))*xscale_bit)))
							{
								index+=2;
							}
							else
							{
								paint = !paint;
								index++;
							}
						}
					}
					if (paint)
					{
						tbyte |=m2sbytes[k];
					}
				}
				tmpbytes[j] = tbyte;
				oled_wdat(tbyte);
			}
		}
		else
		{
			for (j=0; j<(position->w>>1); j++)
			{
				oled_wdat(tmpbytes[j]);
			}
		}
	}
	free(tmpbytes);
}

static inline void oled_program_shownote(const uint8_t *note, Rect_t *position, float xscale_bit,float yscale_bit)
{
	oled_getdataFromPgm(note,180);
	oled_shownote(g_temp_for_program, position, xscale_bit, yscale_bit);
}
static uint8_t oled_show_num(uint32_t num, bool leftJust, float xscale, float yscale, uint8_t width, uint8_t dot_position, Rect_t const * position,bool ispowersave)
{
	uint8_t nbit[5] = {0};
	uint8_t i, numshownd = 0;
	bool firstzero = true;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
	uint8_t currentx = 0;
	uint8_t notesnull[] = {0xff};
	const uint8_t* notes[] = {notes0,notes1,notes2,notes3,notes4,notes5,notes6,notes7,notes8,notes9};
	Rect_t tops = {.x = position->x, .y = position->y, .w = position->w, .h = position->h};
	width = min(width, sizeof(nbit)/sizeof(uint8_t));
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (num/div_nums[i])%10;
		
	}
	for (i = 0;i<5; i++)
	{
		tops.x = position->x +currentx;
		if (i == dot_position)
		{
			firstzero = false;
		}
		if (!firstzero||(nbit[i])!=0)
		{
	//		oled_getdataFromPgm(tmp,notes[nbit[i]]);
			oled_program_shownote(notes[nbit[i]],&tops,xscale,yscale);
			numshownd ++;
			firstzero = false;
			currentx += tops.w;
		}
		else if(!leftJust)
		{
			oled_shownote(notesnull,&tops,xscale,yscale);
			numshownd ++;
			currentx += tops.w;
		}
		if (numshownd == width)
		{
			break;
		}
		if (i == dot_position)
		{
			Rect_t dot_pos;// = {.w = 4,.h = 4,.y=position->y+24};
			if (ispowersave)
			{
				dot_pos.w = 4*xscale;
				dot_pos.h = 4*xscale;
				dot_pos.y=position->y+15;
				currentx += 6;
			}
			else
			{
				dot_pos.w = 4;
				dot_pos.h = 4;
				dot_pos.y=position->y+24;
				currentx += 8;
			}				
			dot_pos.x = tops.x + tops.w + 2;
//			currentx += 8;
			oled_fill(&dot_pos);
		}
	}
	if (firstzero)
	{
//		oled_getdataFromPgm(tmp,notes[0]);
		oled_program_shownote(notes[0],&tops,xscale,yscale);
		if (numshownd == 0)
		{
			numshownd = 1;
		}
	}
	return numshownd;
}


static inline void oled_ic_init(void)
{
		//#if OLED_WHITE
	//uint8_t initlist[] = {0x00,0x10,0x40,0x81,0x14,0xa0,0xa4,0xa6,0xa8,0x3f,0xad,0x8a,0xae,0xc8,0xd3,0x00,0xd5,0x20,0xda,0x12,0xdb,0x30,0xd9,0x22,0xaf};
	//#else
	//uint8_t initlist[] = {0x00,0x10,0x40,0x81,0x20,0xa0,0xa4,0xa6,0xa8,0x3f,0xad,0x8a,0xae,0xc8,0xd3,0x00,0xd5,0x20,0xda,0x12,0xdb,0x00,0xd9,0x22,0xaf};
	//#endif
	uint8_t initlist[] = {0xAE,0xB0,0x00,0x10,0x00,0xd5,0x50,0xd9,0x22,0x40,0x81,0x20,0xa0,0xc8,0xa4,0xa6,0xa8,0x3f,0xad,0x80,0xd3,0x00,0xdb,0x30,0xdc,0x30,0x33,0xaf};
	uint8_t i;
	oled_ic_reset();
	for (i=0;i<sizeof(initlist)/sizeof(uint8_t);i++)
	{
		oled_wcmd(initlist[i]);
	}
	oled_clear_screen();
	
}

static inline void oled_port_init(void)
{
	//	PORTC_DIR = 0xff;//ÉèÖÃÊä³ö
	//	PR_PRPC |= (1<<3);
	//	PORTB_DIR |= (1<<DC)|(1<<RD)|(1<<WR)|(1<<PORT_RES);
	//	PORTA_DIR |= (1<<CS);
	ioport_set_port_dir(OLED_DATA_PORT,0xff,IOPORT_DIR_OUTPUT);
	ioport_set_port_dir(IOPORT_PORTB,0xff,IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(OLED_CS,IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(OLED_ENABLE,IOPORT_DIR_OUTPUT);


}

void oled_init()
{
	oled_port_init();
	oled_ic_init();
}
void oled_inform_check_result(void)
{
	Rect_t position_mode_pre =
	{
		.x = 34,
		.y = 24,
		.w = 70,
		.h = 30,
	};
	//if (stat)
	//{
		//oled_fill(&position_mode_pre);
	//}
	//else
	{
		oled_clear(&position_mode_pre);
	}
}
static void oled_update_battery(uint8_t level, bool ischarge, bool fullscreen)
{
	static uint8_t charge_lvl = 0;
	static uint8_t harf_lvl = false;
	uint8_t display_lvl;
	const uint8_t *batterys[] = {program_img_battery,battery_1,battery_2,battery_3,battery_4};
	if (!ischarge)
	{
		display_lvl = level>>1;
		if (level & 0x01)
		{
			harf_lvl = !harf_lvl;
			if (harf_lvl)
			{
				display_lvl ++;
			}
		}
		charge_lvl = 0;
	}
	else
	{
		if ((charge_lvl)>=5)
		{
			charge_lvl = 0;
		}
		charge_lvl = max(charge_lvl,(level>>1));
		display_lvl = charge_lvl>>0;
		charge_lvl ++;
	}
	
	if (fullscreen)
	{
		oled_program_show_map(batterys[display_lvl],&g_batery_fullscreen_tag,2,sizeof(program_img_battery));
	}
	else
	{
		oled_program_show_map(batterys[display_lvl],&g_batery_tag,0,sizeof(program_img_battery));
	}
}

static void oled_showair_level(uint8_t level, const Rect_t * postion)
{
	const uint8_t * levels[] = {one,two,three,four,five,six};
	const uint8_t size[] = {sizeof(one),sizeof(two),sizeof(three),sizeof(four),sizeof(five),sizeof(six)};
	Rect_t level_pos = *postion;
	Rect_t post_pos = *postion;
	level_pos.y += 0;
	oled_program_show_map(levels[level],&level_pos,0,size[level]);
 	post_pos.x += post_pos.w;
 	post_pos.w = 24;
 	oled_program_show_map(ji,&post_pos,0,sizeof(ji));
}
void oled_show_check_level(uint8_t level)
{
	Rect_t levelpos = {.x = 204,.y = 0,.w = 16,.h = 16};
	const uint8_t * levels[] = {one,two,three,four,five,six};
	const uint8_t size[] = {sizeof(one),sizeof(two),sizeof(three),sizeof(four),sizeof(five),sizeof(six)};
	levelpos.y += 0;
	oled_program_show_map(levels[level],&levelpos,0,size[level]);
// 	levelpos.x += levelpos.w;
// 	oled_program_show_map(ji,&levelpos,0,sizeof(ji));
}
static void oled_showair_description(uint8_t level, const Rect_t * postion)
{
	const uint8_t * description[] = {level_1,level_2,level_3,level_4,level_5,level_6};
	static uint8_t last_level = 0;
	Rect_t tmp_pos = *postion;
// 	if (level<2)
// 	{
// 		tmp_pos.w = tmp_pos.w >>2;
// 		tmp_pos.x += ((tmp_pos.w)>>1)*3;
// 	}
	if (last_level != level)
	{
		last_level = level;
		oled_program_show_map(NULL, postion,0,0);
	}
	oled_program_show_map(description[level],&tmp_pos,0,sizeof(level_4));
}
static inline void oled_power_off_mode(oled_status_t * oled_status)
{
	if (oled_status->show_low_power || oled_status->is_charge)
	{
		oled_update_battery(oled_status->battery_level, oled_status->is_charge, true);
	}
	else
	{
		oled_clear_screen();
	}	
}

static inline void oled_realtime_mode(oled_status_t * oled_status)
{
	if (oled_status->is_hold)
	{
		//oled_clear(&g_mode_tag);//20140327
		oled_program_show_map(program_img_hold, &g_hold_tag,0,sizeof(program_img_hold));
		//oled_showair_level(oled_status->aqi_level,oled_status->is_hold);
	}
	else
	{
		//oled_clear(&g_hold_tag);
		oled_program_show_map(program_img_realtime_mode, &g_mode_tag,0,sizeof(program_img_realtime_mode));
	}
	oled_program_show_map(pm25_big, &g_real_time_pm25_tag,0,sizeof(pm25_big));
	oled_program_show_map(ug_big, &g_real_time_pm25_unit,0,sizeof(ug_big));
	oled_show_num(oled_status->pm2_5,false,0.75,0.9,5,2,&g_real_time_pm25_num,false);
	oled_update_battery(oled_status->battery_level, oled_status->is_charge, false);
	oled_showair_description(oled_status->aqi_level,&g_aqi_description_realtime);
}

static inline void oled_powersave_mode(oled_status_t * oled_status)
{
	Rect_t pm25_tag =
	{
		.x = 32,
		.y = 6,
		.w = 44,
		.h = 9,
	};
	Rect_t pm25_num =
	{
		.x = 76,
		.y = 10,
		.w = 12,
		.h = 20,
	};
	Rect_t pm25_unit =
	{
		.x = 36,
		.y = 20,
		.w = 32,
		.h = 10,
	};
	Rect_t pm10_tag =
	{
		.x = 150,
		.y = 6,
		.w = 40,
		.h = 9,
	};
	Rect_t pm10_num =
	{
		.x = 190,
		.y = 10,
		.w = 12,
		.h = 20,
	};
	Rect_t pm10_unit =
	{
		.x = 154,
		.y = 20,
		.w = 32,
		.h = 10,
	};
	Rect_t g_aqi_discription_pre =
	{
		.x = 33,
		.y = 40,
		.w = 96,
		.h = 16,
	};
	//oled_clear_screen();
	Rect_t line_row = {.x = 32,.y = 32,.w = 224,.h = 1};
	Rect_t line_colum = {.x = 148,.y = 0,.w = 1,.h = 32};
	const static Rect_t aqipos = {.x = 134,.y = 38,.w = 8,.h = 16};
	oled_draw_rectangle(&line_row,0xff);
	oled_draw_rectangle(&line_colum,0xcc);
	oled_program_show_map(pm25_small,&pm25_tag,0,sizeof(pm25_small));
	oled_program_show_map(ug_small,&pm25_unit,0,sizeof(ug_small));
	oled_program_show_map(ug_small,&pm10_unit,0,sizeof(ug_small));
	oled_program_show_map(pm10,&pm10_tag,0,sizeof(pm10));
	oled_program_show_map(leveldes,&g_aqi_discription_pre,0,sizeof(leveldes));
	
	oled_showair_level(oled_status->aqi_level,&aqipos);
	oled_showair_description(oled_status->aqi_level,&g_aqi_description_powersave);
	oled_show_num(oled_status->pm2_5,false,0.5,0.54,5,2,&pm25_num,true);
	oled_show_num(oled_status->pm10,false,0.5,0.54,5,2,&pm10_num,true);
	oled_update_battery(oled_status->battery_level, oled_status->is_charge, false);
}

static inline void oled_history_mode(oled_status_t * oled_status)
{
	
	Rect_t position_mode_pre =
	{
		.x = 40,
		.y = 0,
		.w = 72,
		.h = 16,
	};
	Rect_t position_number  =
	{
		.x  = 112,
		.y  = 0,
		.w = 12,
		.h = 16,
	};
	Rect_t position_mode_post =
	{
		.x = 0,
		.y = 0,
		.w = 32,
		.h = 16,
	};
	
	oled_program_show_map(program_img_histroy_mode,&position_mode_pre,0,sizeof(program_img_histroy_mode));
	
	oled_update_battery(oled_status->battery_level, oled_status->is_charge, false);
	
	oled_program_show_map(pm25_big, &g_real_time_pm25_tag,0,sizeof(pm25_big));

	oled_program_show_map(ug_big, &g_real_time_pm25_unit,0,sizeof(ug_big));
	oled_show_num(oled_status->pm2_5,false,0.75,0.9,5,2,&g_real_time_pm25_num,false);
	{
		uint32_t time;
		uint32_t subtime;
		uint8_t x_offset = 0;
		uint8_t i;
		const static uint8_t div_for_time[] = {60,60,24,30};
		const static uint8_t * pointers[] = {time_second, time_minute, time_hour, time_day, time_month};	
		const static uint8_t timeoffset[] = {8,24,8,24,8};
		time = oled_status->time;
		for (i=0; i<3; i++)
		{
			subtime = time%div_for_time[i];
			time = time/div_for_time[i];
			if (time < div_for_time[i+1])
			{
				break;
			}
		}
		
		time = min(time,99);
		if (time>0)
		{
			Rect_t pos_tmp = position_mode_post;
			x_offset += position_number.w * oled_show_num(time,true,0.44,0.44,2,10,&position_number,false);
			pos_tmp.x = position_number.x + x_offset;
			pos_tmp.w = timeoffset[i+1];
			oled_program_show_map(pointers[i+1], &pos_tmp,0,sizeof(time_minute));
			x_offset += timeoffset[i+1];
		}
		{
			Rect_t pos_tmp = position_number;
			pos_tmp.x = position_number.x + x_offset;
			x_offset += position_number.w * oled_show_num(subtime,true,0.44,0.44,2,10,&pos_tmp,false);
		}			
		{	
			Rect_t pos_tmp = position_mode_post;
			pos_tmp.x = position_number.x + x_offset;
			pos_tmp.w = timeoffset[i];
			oled_program_show_map(pointers[i], &pos_tmp,0,sizeof(time_minute));
			x_offset += timeoffset[i];
		}
		if (x_offset<112)
		{	
			Rect_t pos_tmp = position_mode_post;
			pos_tmp.x = position_number.x + x_offset;
			oled_program_show_map(program_img_history_post, &pos_tmp,0,sizeof(program_img_history_post));
			x_offset += 32;
		}
		if (x_offset<116)
		{
			Rect_t pos_tmp = position_number;
			pos_tmp.x += x_offset;
			pos_tmp.w = 116 - x_offset;
			oled_clear(&pos_tmp);
		}
	}
	
}

void oled_update(oled_status_t * oled_status)
{
	system_status_t ss = oled_status->system_status;
	static system_status_t last_system_status = SYSTEM_POWERDOFF;
	if (last_system_status != ss)
	{
		oled_clear_screen();
		last_system_status = ss;
	}
// 	if (oled_status->pm2_5 >= 99999)
// 	{
// 		oled_status->pm2_5 = 99999;
// 	}
	switch(ss)
	{
		case SYSTEM_REALTIME:
			oled_realtime_mode(oled_status);
		break;
		case SYSTEM_POWERSAVE:
			oled_powersave_mode(oled_status);
			break;
		case SYSTEM_HISTORY:
			oled_history_mode(oled_status);
			break;
		case SYSTEM_POWERDOFF:
			oled_power_off_mode(oled_status);
			break;
		default:
		
		break;
	}		
			
}