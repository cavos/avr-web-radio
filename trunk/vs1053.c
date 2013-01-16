/*
 * vs1053.c
 *
 * Created: 2013-01-15 13:04:43
 *  Author: krzychu
 */ 
#include "vs1053.h"

INT8	_vs_trebleAmp = VS_DEF_TEBAMP;
UINT8	_vs_trebleFreq = VS_DEF_TEBFREQ;
UINT8	_vs_bassAmp = VS_DEF_BASSAMP;
UINT8	_vs_bassFreq = VS_DEF_BASSFREQ;
UINT8	_vs_volume = VS_DEF_VOLUME;

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
	VS_XCS_ENABLE();
	
	spi_write(VS_READOP);
	spi_write(addr);
	value = value | spi_read()<<8;
	value = value | spi_read();
	
	VS_XCS_DISABLE();
	
	while(vsCheckDreq() == 0)
	{}
	
	return value;
}

void	vsWriteReg(UINT8 addr, UINT16 value)
{
	VS_XCS_ENABLE();
	
	spi_write(VS_WRITEOP);
	spi_write(addr);
	spi_write(value>>8);
	spi_write(value&0x00FF);
	
	VS_XCS_DISABLE();
	
	while(vsCheckDreq() == 0)
	{}
}

void	vsSoftReset()
{
	VS_XCS_ENABLE();
	
	spi_write(VS_WRITEOP);
	spi_write(VS_SCI_MODE);
	spi_write(VS_SM_RESET>>8);
	spi_write(VS_SM_RESET);
	
	VS_XCS_DISABLE();
	
	while(vsCheckDreq() == 0){}
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
	
	vsWriteReg(VS_SCI_VOL, ((_vs_volume<<8)|(_vs_volume)));
}

UINT8	vsCheckDreq()
{
	return (VS_DREQ)?0xFF:0x00;
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
	vsWriteReg(VS_SCI_MODE, VS_SM_RESET);
	
	while(vsCheckDreq() == 0){};
		
	vsInitialize();
}

void	vsInitialize()
{
	
}

void	vsPatch()
{
	
}