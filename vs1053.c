/*
 * vs1053.c
 *
 * Created: 2013-01-15 13:04:43
 *  Author: krzychu
 */ 
#include "vs1053.h"
#include <util/delay.h>
#include "fifo.h"

INT8	_vs_trebleAmp;
UINT8	_vs_trebleFreq;
UINT8	_vs_bassAmp;
UINT8	_vs_bassFreq;
UINT8	_vs_volume;

void	vsWriteBass()
{
	UINT16 tmp = 0;
	tmp = (_vs_trebleAmp&0x0F)<<12;
	tmp = tmp | (_vs_trebleFreq&0x0F)<<8;
	tmp = tmp | (_vs_bassAmp&0x0F)<<4;
	tmp = tmp | (_vs_bassFreq&0x0F);
	vsWriteReg(VS_SCI_BASS, tmp);
}

UINT16	vsReadReg(UINT8 addr)
{
	UINT16 value = 0;
	spi_2x_clr();
	VS_XCS_ENABLE();
	
	spi_write(VS_READOP);
	spi_write(addr);
	value = value | spi_read()<<8;
	value = value | spi_read();
	
	VS_XCS_DISABLE();
	spi_2x();
	
	while(vsCheckDreq() == 0)
	{}
	
	return value;
}

void	vsWriteReg(UINT8 addr, UINT16 value)
{
	spi_2x_clr();
	VS_XCS_ENABLE();
	
	spi_write(VS_WRITEOP);
	spi_write(addr);
	spi_write(value>>8);
	spi_write(value&0xFF);
	
	VS_XCS_DISABLE();
	spi_2x();
	
	
	while(vsCheckDreq() == 0)
	{}
		VS_XCS_DISABLE();
}

UINT16	vsGetTrebleFreq()
{
	UINT16 trebFreq = 0;
	
	trebFreq = _vs_trebleFreq*1000; // treble freq is stored as 1..15 value
	
	return trebFreq; // return as 1k..15k Hz value
}

INT8	vsSetTrebleFreq(UINT16 tFreq)
{
	if (tFreq < 1000 || tFreq > 15000)
	{
		return -1;
	}
	
	tFreq = tFreq%1000; // treble frequency is stored as 1..15 value
	_vs_trebleFreq = tFreq;
	
	vsWriteBass();
	
	return 0;
}

INT8	vsGetTrebleAmp()
{
	return _vs_trebleAmp;
}

void	vsSetTrebleAmp(INT8 tAmp)
{
	if (tAmp < -8)
	{
		tAmp = -8;
	}
	else if (tAmp > 7)
	{
		tAmp = 7;
	}
	
	_vs_trebleAmp = tAmp;
	vsWriteBass();
}

UINT8	vsGetBassFreq()
{
	return _vs_bassFreq*10;
}

void	vsSetBassFreq(UINT8 bFreq)
{
	if(bFreq > 150)
	{
		bFreq = 150;
	}
	else if(bFreq < 20)
	{
		bFreq = 20;
	}
	
	_vs_bassFreq = bFreq;
	vsWriteBass();
}

UINT8	vsGetBassAmp()
{
	return _vs_bassAmp;
}

void	vsSetBassAmp(UINT8 bAmp)
{
	if (bAmp > 15)
	{
		bAmp = 15;
	}
	
	_vs_bassAmp = bAmp;
	vsWriteBass();
}

UINT8	vsGetVolume()
{
	return _vs_volume;
}

void	vsSetVolume(UINT8 volume)
{
	if (volume > 100)
	{
		volume = 100;
	}
	
	UINT16 vol = 0x0000;
	_vs_volume = volume;
	
	vol = 100 - _vs_volume;
	
	vsWriteReg(VS_SCI_VOL, ((vol<<8)|(vol)));
}

UINT8	vsCheckDreq()
{
	return (VS_DREQ())?0xFF:0x00;
}

UINT8	vsData(UINT8 *data)
{
	int i = 0;
	VS_XDCS_ENABLE();
	for(i = 0; i < 32; i++)
	{
		spi_write(*data);
		data++;
	}
	VS_XDCS_DISABLE();
	
	return 0;
}

void	vsCancel()
{
	UINT8 tmp = 0;
	tmp = vsReadReg(VS_SCI_MODE);
	tmp = tmp|(VS_SM_CANCEL);
	vsWriteReg(VS_SCI_MODE, tmp);
}

void	vsSoftReset()
{	
	vsPutZeros(2052);

	vsWriteReg(VS_SCI_MODE, (VS_SM_EARSPKLO|VS_SM_SDINEW|VS_SM_RESET));
	//_delay_ms(2);	
	//
}

void	vsInitialize()
{
	vsSoftReset();
	
	vsSetBassAmp(VS_DEF_BASSAMP);
	vsSetBassFreq(VS_DEF_BASSFREQ);
	vsSetTrebleAmp(VS_DEF_TEBAMP);
	vsSetTrebleFreq(VS_DEF_TEBFREQ);
	vsWriteReg(VS_SCI_AUDATA, 44101); // 44kHz samplerate, stereo
	vsWriteReg(VS_SCI_MODE, (VS_SM_EARSPKLO |/* VS_SM_STREAM |*/ VS_SM_SDINEW));
	
	vsWriteReg(VS_SCI_STATUS, 0x000E); // ss_ref - 1.65V
	vsWriteReg(VS_SCI_CLOCKF, 0xa800); // SC_MULT=4x, SC_ADD=1.0x see datasheet
	vsSetVolume(VS_DEF_VOLUME);
	
	//vsSineTest(126, 100);
	vsPutZeros(32);
	VS_XDCS_ENABLE();
	spi_write(0xFF);
	spi_write(0xF2);
	spi_write(0x40);
	spi_write(0xC0);
	VS_XDCS_DISABLE();
}

void	delay_ms(UINT8 length)
{
	while(length--)
	{
		_delay_ms(1);
	}
}

void	vsSineTest(UINT8 FsS, UINT16 length)
{
	vsWriteReg(VS_SCI_MODE, (VS_SM_EARSPKLO| VS_SM_TESTS|VS_SM_SDINEW));
	
	vsWriteReg(VS_SCI_VOL, 0x00FE);
	
	VS_XDCS_ENABLE(); // select data pin
	spi_write(0x53);  // test sequence activation
	spi_write(0xEF);
	spi_write(0x6E);
	spi_write(FsS);   // pitch
	vsPutZeros(4);    // zeros
	VS_XDCS_DISABLE();// deselect data pin
	
	delay_ms(length); 
	
	VS_XDCS_ENABLE(); // select data ping
	spi_write(0x45);  // test sequence deactivation
	spi_write(0x78);
	spi_write(0x69);
	spi_write(0x74);
	vsPutZeros(4);    // zeros
	VS_XDCS_DISABLE();// deselect data pin
	
	vsWriteReg(VS_SCI_VOL, 0xFE00);
	
	VS_XDCS_ENABLE();
	spi_write(0x53);
	spi_write(0xEF);
	spi_write(0x6E);
	spi_write(FsS);
	vsPutZeros(4);
	VS_XDCS_DISABLE();
	
	delay_ms(length);
	
	VS_XDCS_ENABLE();
	spi_write(0x45);
	spi_write(0x78);
	spi_write(0x69);
	spi_write(0x74);
	vsPutZeros(4);
	VS_XDCS_DISABLE();
}

void	vsPutZeros(UINT16 count)
{
	spi_2x_clr();
	VS_XDCS_ENABLE();
	while(count--)
	{
		SPDR = 0x00;
		wait_spi();
	}
	VS_XDCS_DISABLE();
	spi_2x();
}

void vsLoadUserCodeOld( UINT8 atab[], UINT16 dtab[], UINT32 size )
{
	UINT32 index = 0;
	while(size--)
	{
		vsWriteReg(atab[index], dtab[index]);
		index++;
	}
}

void	vsLoadPlg(UINT16 plg[], UINT16 size)
{
	UINT32 i = 0;
	UINT16 addr, n, value;
	
	while(i < size)
	{
		addr = plg[i++];
		n = plg[i++];
		
		if(n & 0x8000)// RLE run, replicate n samples
		{
			n &= 0x7FFF;
			value = plg[i++];
			
			while(n--)
			{
				vsWriteReg(addr, value);
			}
		}
		else // copy run, copy n samples
		{
			while(n--)
			{
				value = plg[i++];
				vsWriteReg(addr, value);
			}
		}
	}
}