#ifndef HYDROPONICS_LCD_DEV_ST7796S_REGS_H
#define HYDROPONICS_LCD_DEV_ST7796S_REGS_H

typedef enum {
    ST7796S_REG_NOP = 0x00,          /*!< No operation */
    ST7796S_REG_SWRESET = 0x01,      /*!< Software reset */
    ST7796S_REG_RDDID = 0x04,        /*!< Read display ID */
    ST7796S_REG_RDDSI_ERRORS = 0x05, /*!< Read DSI */
    ST7796S_REG_RDDST = 0x09,        /*!< Read display status */
    ST7796S_REG_RDDPM = 0x0a,        /*!< Read display power */
    ST7796S_REG_RDDMADCTL = 0x0b,    /*!< Read display */
    ST7796S_REG_RDDCOLMOD = 0x0c,    /*!< Read display pixel */
    ST7796S_REG_RDDIM = 0x0d,        /*!< Read display image */
    ST7796S_REG_RDDSM = 0x0e,        /*!< Read display signal */
    ST7796S_REG_RDDSDR = 0x0f,       /*!< Read display self-diagnostic result */
    ST7796S_REG_SLPIN = 0x10,        /*!< Sleep in */
    ST7796S_REG_SLPOUT = 0x11,       /*!< Sleep out */
    ST7796S_REG_PTLON = 0x12,        /*!< Partial mode on */
    ST7796S_REG_NORON = 0x13,        /*!< Partial off (Normal) */
    ST7796S_REG_INVOFF = 0x20,       /*!< Display inversion off */
    ST7796S_REG_INVON = 0x21,        /*!< Display inversion on */
    ST7796S_REG_DISPOFF = 0x28,      /*!< Display off */
    ST7796S_REG_DISPON = 0x29,       /*!< Display on */
    ST7796S_REG_CASET = 0x2a,        /*!< Column address set */
    ST7796S_REG_RASET = 0x2b,        /*!< Row address set */
    ST7796S_REG_RAMWR = 0x2c,        /*!< Memory write */
    ST7796S_REG_RAMRD = 0x2e,        /*!< Memory read */
    ST7796S_REG_PTLAR = 0x30,        /*!< Partial start/end address set */
    ST7796S_REG_VSCRDEF = 0x33,      /*!< Vertical scrolling definition */
    ST7796S_REG_TEOFF = 0x34,        /*!< Tearing effect line off */
    ST7796S_REG_TEON = 0x35,         /*!< Tearing effect line on */
    ST7796S_REG_MADCTL = 0x36,       /*!< Memory data access control */
    ST7796S_REG_VSCSAD = 0x37,       /*!< Vertical scrolling start address */
    ST7796S_REG_IDMOFF = 0x38,       /*!< Idle mode off */
    ST7796S_REG_IDMON = 0x39,        /*!< Idle mode on */
    ST7796S_REG_COLMOD = 0x3a,       /*!< Interface pixel format */
    ST7796S_REG_WRMEMC = 0x3c,       /*!< Memory write continue */
    ST7796S_REG_RDMEMC = 0x3e,       /*!< Memory read continue */
    ST7796S_REG_STE = 0x44,          /*!< Set tear scanline */
    ST7796S_REG_GSCAN = 0x45,        /*!< Get scanline */
    ST7796S_REG_WRDISBV = 0x51,      /*!< Write display brightness */
    ST7796S_REG_RDDISBV = 0x52,      /*!< Read display brightness value */
    ST7796S_REG_WRCTRLD = 0x53,      /*!< Write CTRL display */
    ST7796S_REG_RDCTRLD = 0x54,      /*!< Read CTRL value dsiplay */
    ST7796S_REG_WRCABC = 0x55,       /*!< Write content adaptive brightness control */
    ST7796S_REG_RDCABC = 0x56,       /*!< Read content adaptive brightness control */
    ST7796S_REG_WRCABCMB = 0x5e,     /*!< Write CABC minimum brightness */
    ST7796S_REG_RDCABCMB = 0x5f,     /*!< Read CABC minimum brightness */
    ST7796S_REG_RDFCS = 0xaa,        /*!< Read First Checksum */
    ST7796S_REG_RDCFCS = 0xaf,       /*!< Read Continue Checksum */
    ST7796S_REG_RDID1 = 0xda,        /*!< Read ID1 */
    ST7796S_REG_RDID2 = 0xdb,        /*!< Read ID2 */
    ST7796S_REG_RDID3 = 0xdc,        /*!< Read ID3 */
    ST7796S_REG_IFMODE = 0xb0,       /*!< Interface Mode Control */
    ST7796S_REG_FRMCTR1 = 0xb1,      /*!< Frame Rate Control（In Normal Mode/Full Colors） */
    ST7796S_REG_FRMCTR2 = 0xb2,      /*!< Frame Rate Control（In Idle Mode/8 colors） */
    ST7796S_REG_FRMCTR3 = 0xb3,      /*!< Frame Rate Control（In Partial Mode/Full colors） */
    ST7796S_REG_DIC = 0xb4,          /*!< Display Inversion Control */
    ST7796S_REG_BPC = 0xb5,          /*!< Blanking Porch Control */
    ST7796S_REG_DFC = 0xb6,          /*!< Display Function Control */
    ST7796S_REG_EM = 0xb7,           /*!< Entry Mode Set */
    ST7796S_REG_PWR1 = 0xc0,         /*!< Power Control 1 */
    ST7796S_REG_PWR2 = 0xc1,         /*!< Power Control 2 */
    ST7796S_REG_PWR3 = 0xc2,         /*!< Power Control 3 */
    ST7796S_REG_VCMPCTL = 0xc5,      /*!< Vcom Control */
    ST7796S_REG_VCM_OFFSET = 0xc6,   /*!< Vcom Offset Register */
    ST7796S_REG_NVMADW = 0xd0,       /*!< NVM Address/Data */
    ST7796S_REG_NVMBPROG = 0xd1,     /*!< NVM Byte Program Control */
    ST7796S_REG_NVMSTRD = 0xd2,      /*!< NVM Status Read */
    ST7796S_REG_RDID4 = 0xd3,        /*!< Read ID4 */
    ST7796S_REG_PGC = 0xe0,          /*!< Positive Gamma Control */
    ST7796S_REG_NGC = 0xe1,          /*!< Negative Gamma Control */
    ST7796S_REG_DGC1 = 0xe2,         /*!< Digital Gamma Control1 */
    ST7796S_REG_DGC2 = 0xe3,         /*!< Digital Gamma Control2 */
    ST7796S_REG_DOCA = 0xe8,         /*!< Display Output CTRL Adjust */
    ST7796S_REG_CSCON = 0xf0,        /*!< Command Set Control */
    ST7796S_REG_SPIRC = 0xfb,        /*!< SPI Read Control */
} st7796s_reg_t;

typedef enum {
    ST7796S_MADCTL_MY = BIT(7),
    ST7796S_MADCTL_MX = BIT(6),
    ST7796S_MADCTL_MV = BIT(5),
    ST7796S_MADCTL_ML = BIT(4),
    ST7796S_MADCTL_BGR = BIT(3),
    ST7796S_MADCTL_MH = BIT(2),
    ST7796S_MADCTL_RGB = 0,
} st7796s_madctl_t;

typedef enum {
    ST7796S_DSI_ERROR_SOT = BIT(0),
    ST7796S_DSI_ERROR_SOT_SYNC = BIT(1),
    ST7796S_DSI_ERROR_EOT_SYNC = BIT(2),
    ST7796S_DSI_ERROR_ESCAPE_MODE_ENTRY_COMMAND = BIT(3),
    ST7796S_DSI_ERROR_LOW_POWER_TRANSMIT_SYNC = BIT(4),
    ST7796S_DSI_ERROR_HS_RECEIVE_TIMEOUT = BIT(5),
    ST7796S_DSI_ERROR_FALSE_CONTROL = BIT(6),
    ST7796S_DSI_ERROR_RESERVED1 = BIT(7),
    ST7796S_DSI_ERROR_ECC_ERROR_SINGLE_BIT_DETECTED_AND_CORRECTED = BIT(8),
    ST7796S_DSI_ERROR_ECC_ERROR_MULTI_BIT_DETECTED_NOT_CORRECTED = BIT(9),
    ST7796S_DSI_ERROR_CHECKSUM_ERROR_LONG_PACKET_ONLY = BIT(10),
    ST7796S_DSI_ERROR_DSI_DATA_TYPE_NOT_RECOGNIZED = BIT(11),
    ST7796S_DSI_ERROR_DSI_VC_ID_INVALID = BIT(12),
    ST7796S_DSI_ERROR_INVALID_TRANSMISSION_LENGTH = BIT(13),
    ST7796S_DSI_ERROR_RESERVED2 = BIT(14),
    ST7796S_DSI_ERROR_DSI_PROTOCOL_VIOLATION = BIT(15),
} st7796s_dsi_error_t;

#endif //HYDROPONICS_LCD_DEV_ST7796S_REGS_H
