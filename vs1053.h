/*
 * vs1053.h
 *
 * Created: 2013-01-10 20:05:11
 *  Author: krzychu
 * 
 * TODO:
 * - return codes
 * - 
 */ 


#ifndef VS1053_H_
#define VS1053_H_

#include "main.h"
#include "types.h"

// defaults ----------------------------------------------------------------
#define VS_DEF_VOLUME	(50)	// <0,100>%
#define VS_DEF_BASSAMP	(5)		// <0,15>dB
#define VS_DEF_BASSFREQ	(100)	// <20-150>Hz
#define VS_DEF_TEBFREQ	(12000) // <1k-15k>Hz
#define VS_DEF_TEBAMP	(0)		// <-8,7>dB

//patch -------------------------------
//#define VS_PATCH_WOFLAC (2095)
//extern	const unsigned char atab[VS_PATCH_WOFLAC];
//const unsigned short dtab[VS_PATCH_WOFLAC];


// op commands -------------------------------------------------------------
#define	VS_READOP	(0x03)
#define	VS_WRITEOP	(0x02)

// SCI registers ----------------------------------------------------------
#define	VS_SCI_MODE			(0x00) // mode control
	#define VS_SM_DIFF		(0x0001) // Differential[ 0 - normal in-phase audio, 1 - left channel inverted]
	#define VS_SM_LAYER12	(0x0002) // Allow MPEG layers I & II [0- no, 1 - yes]
	#define VS_SM_RESET		(0x0004) // Soft reset [0 - no reset, 1 - reset]
	#define VS_SM_CANCEL	(0x0008) // Cancel decoding current file [0 - no, 1 - yes]
	#define VS_SM_EARSPKLO	(0x0010) // EarSpeaker low setting [0 - off, 1 - active]
	#define VS_SM_TESTS		(0x0020) // Allow SDI tests [0 - not allowed, 1 - allowed]
	#define VS_SM_STREAM	(0x0040) // Stream mode [0 - no, 1 - yes]
	#define VS_SM_EARSPKHI  (0x0080) // EarSpeaker high setting [0 - off, 1 - active]
	#define VS_SM_DACT		(0x0100) // DCLK active edge [0 - rising, 1 - falling]
	#define VS_SM_SDIORD	(0x0200) // SDI bit order [0 - MSb first, 1 LSb first]
	#define VS_SM_SDISHARE  (0x0400) // Share SPI chip select [0 - no, 1 - yes]
	#define VS_SM_SDINEW	(0x0800) // VS1002 native SPI modes [0 - no, 1 - yes]
	#define VS_SM_ADPCM		(0x1000) // PCM/ADPCM recording active [0 - no, 1- yes]
	#define VS_SM_LINE1		(0x4000) // MIC/LINE1 selector [0 - MICP, 1 - LINE1]
	#define VS_SM_CLK_RANGE (0x8000) // Input clock range [0 - 12..13 MHz, 1 - 24..26 MHz]
#define VS_SCI_STATUS		(0x01) // status of VS1053b
	#define VS_SS_DONOTJUMP (0x8000) // Header in decode, do not fast forward/rewind
	#define VS_SS_SWING		(0x7000) // Set swing to +0 dB, +0.5 dB, .., or +3.5 dB
	#define VS_VCM_OVLOAD	(0x0800) // GBUF overload indicator [1 - overload]
	#define VS_VCM_DISABLE	(0x0400) // GBUF overload detection [1 - disable]
	#define VS_VER			(0x00F0) // Version
	#define VS_APDOWN2		(0x0008) // Analog driver powerdown
	#define VS_APDOWN1		(0x0004) // Analog internal powerdown
	#define VS_AD_CLOCK		(0x0002) // AD clock select [0 - 6MHz, 1 - 3MHz]
	#define VS_REF_SEL		(0x0001) // Reference voltage selection [0 - 1.23V, 1 - 1.65V]
#define VS_SCI_BASS			(0x02) // Built-in bass/treble control
	#define VS_ST_AMP		(0xF000) // Treble Control in 1.5 dB steps (-8..7, 0 = off)
	#define VS_ST_FREQLIM	(0x0F00) // Lower limit frequency in 1000 Hz steps (1..15)
	#define VS_SB_AMP		(0x00F0) // Bass Enchancement in 1 dB steps (0..15, 0 = off)
	#define VS_SB_FREQLIM	(0x000F) // Lower limit frequnecy in 10Hz steps (2..15)
#define VS_SCI_CLOCKF		(0x03) // Clock freq + multiplier
	#define VS_SC_MULT		(0xE000) // Clock multiplier
	#define VS_SC_MULTx0	(0x0000) // XTALI
	#define VS_SC_MULTx1	(0x2000) // XTALIx2.0
	#define VS_SC_MULTx2	(0x4000) // XTALIx2.5
	#define VS_SC_MULTx3	(0x6000) // XTALIx3.0
	#define VS_SC_MULTx4	(0x8000) // XTALIx3.5
	#define VS_SC_MULTx5	(0xa000) // XTALIx4.0
	#define VS_SC_MULTx6	(0xc000) // XTALIx4.5
	#define VS_SC_MULTx7	(0xe000) // XTALIx5.0
	#define VS_SC_ADD		(0x1800) // Allowed multiplier addition
	#define VS_SC_ADDx0		(0x0000) // No modification allowed
	#define VS_SC_ADDx1		(0x0800) // XTALIx1.0
	#define VS_SC_ADDx2		(0x1000) // XTALIx1.5
	#define VS_SC_ADDx3		(0x1800) // XTALIx2.0
	#define VS_SC_FREQ		(0x04FF) // Clock frequency
#define VS_SCI_DECODETIME	(0x04) // Decode time in seconds
#define VS_SCI_AUDATA		(0x05) // Misc. audio data
#define VS_SCI_WRAM			(0x06) // RAM write/read
#define VS_SCI_WRAMADDR		(0x07) // Base address for RAM read/write
#define VS_SCI_HDAT0		(0x08) // Stream header data 0
	#define VS_HD0_BITRATE	(0xF000) // see bitrate table
	#define VS_HD0_SAMPLER	(0x0C00) // 3 - reserved, 2 - 32/16/8 kHz, 1 - 48/24/12 kHz, 0 - 44/22/11 kHz
	#define VS_HD0_PADD		(0x0200) // pad bit, 1 - additional slot, 0 - normal frame
	#define VS_HD0_MODE		(0x00C0) // 3 - mono, 2 - dual channel, 1 - joint stereo, 0 - stereo
	#define VS_HD0_EXT		(0x0030) // see ISO 11172-3
	#define VS_HD0_COPYRIG	(0x0008) // 1 - copyrighted, 0 - free
	#define VS_HD0_ORIGINAL	(0x0004) // 1 - original, 0 - copy
	#define VS_HD0_EMPHASIS	(0x0003) // 3 - CCITT J.17, 2 - reserved, 1 - 50/15 microsec, 0 -none
#define VS_SCI_HDAT1		(0x09) // Stream header data 1
	#define VS_HD1_SYNWRD	(0xFFF0) // value = 2047, stream valid
	#define VS_HD1_ID		(0x000C) // 3 - MPG 1.0, 2 - MPG 2.0 (1/2-rate), 1 - MPG 2.5 (1/4-rate), 0 - MPG 2.5 (1/4-rate)
	#define VS_HD1_LAYER	(0x0003) // 3 - I, 2 - II, 1 - III, 0 - reserved
#define VS_SCI_AIADDR		0x0A // Start address of application
#define VS_SCI_VOL			0x0B // Volume control
#define VS_SCI_AICTRL0		0x0C // Application control register 0
#define VS_SCI_AICTRL1		0x0D // Application control register 1
#define VS_SCI_AICTRL2		0x0E // Application control register 2
#define VS_SCI_AICTRL3		0x0F // Application control register 3

#define VS_EARSPK_OFF		(0)
#define VS_EARSPK_MIN		(1)
#define VS_EARSPK_NORMAL	(2)
#define VS_EARSPK_EXTR		(3)


// --------------------------------------------------------------------------
///<summary>
/// Read treble frequency
/// returns treble frequency <1k,15>kHz
///</summary>
UINT16	vsGetTrebleFreq();

///<summary>
/// Set treble frequency
/// UINT8 tFreq - new treble frequency
/// return 0 if successful, -1 otherwise
///</summary>
INT8	vsSetTrebleFreq(UINT16 tFreq);

///<summary>
/// Read treble amplitude
/// return treble amplitude <-8,7>dB
///</summary>
INT8	vsGetTrebleAmp();

///<summary>
/// Set treble amplitude
/// INT8 tAmp - new treble amplitude <-8,7>dB
///</summary>
void	vsSetTrebleAmp(INT8 tAmp);

///<summary>
/// Read bass frequency
/// return bass frequency <20,150>Hz
///</summary>
UINT8	vsGetBassFreq();

///<summary>
/// Set bass frequency
/// UINT8 bFreq - new bass frequency <20,150>Hz
///</summary>
void	vsSetBassFreq(UINT8 bFreq);

///<summary>
/// Read bass amplitude
/// Returns bass amplitude <-8,7>dB
///</summary>
UINT8	vsGetBassAmp();

///<summary>
/// Set bass amplitude
/// UINT8 bass - new bass amplitude <-8,7>dB
///</summary>
void	vsSetBassAmp(UINT8 bAmp);

///<summary>
/// Read volume value
/// Return volume value
///</summary>
UINT8	vsGetVolume();

///<summary>
/// Set volume value, both channels get the same value
/// UINT8 volume value
///</summary>
void	vsSetVolume(UINT8 volume);

///<summary>
/// Send 32 bytes of data to vs
/// UINT8 *data - pointer to 32 chunks of data
/// return 0 if successful, 255 otherwise
///</summary>
UINT8	vsData(UINT8 *data);

///<summary>
/// Cancels current playback
///</summary>
void	vsCancel();

///<summary>
///	Apply patch
///</summary>
//void	vsPatch();

///<summary>
/// Reset vs1053
///</summary>
void	vsSoftReset();

///<summary>
/// Initializes and applies patch
///</summary>
void	vsInitialize();

///<summary>
/// Read register value
/// UINT16 addr - register address
/// return register value
///</summary>
UINT16	vsReadReg(UINT8 addr);

///<summary>
/// Writes to selected register
/// UINT16 addr - register addr
/// UINT16 value - register new value
///</summary>
void	vsWriteReg(UINT8 addr, UINT16 value);

///<summary>
/// Gets dreq level
/// return 0 if low, 255 if high
///</summary>
UINT8	vsCheckDreq();

///<summary>
/// Put series of zeros 
/// UINT16 count - number of zeros
///</summary>
void	vsPutZeros(UINT16 count);

///<summary>
/// Sine test
/// UINT8 FsS - see datasheet 9.12.1
/// UINT16 length - duraton of test in ms
///</summary>
void	vsSineTest(UINT8 FsS, UINT16 length);

#endif /* VS1053_H_ */